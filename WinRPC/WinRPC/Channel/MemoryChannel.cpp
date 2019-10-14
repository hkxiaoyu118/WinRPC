#include "MemoryChannel.h"
#include <process.h>

MemoryChannel::MemoryChannel(const std::string channelName, bool isServer, DWORD shareMemorySize, unsigned sendMaxSize, unsigned receiveMaxSize, FunProcessRecvData* pFunc, void* pContext)
{
	m_hSendThread = NULL;
	m_hReceiveThread = NULL;
	m_channelName = channelName;
	m_shareMemorySize = shareMemorySize;
	m_isServer = isServer;
	m_sendMaxSize = sendMaxSize;
	m_receiveMaxSize = receiveMaxSize;
	m_pRecvDataFunc = pFunc;
	m_pContext = pContext;
	InitializeCriticalSection(&m_dataCs);
	InitializeCriticalSection(&m_sendCs);
}

MemoryChannel::~MemoryChannel()
{
	//先停止收发工作线程
	m_threadWorking = false;
	HANDLE hHandles[2] = { m_hSendThread, m_hReceiveThread };
	WaitForMultipleObjects(2, hHandles, TRUE, INFINITE);
	CloseHandle(m_hSendThread);
	CloseHandle(m_hReceiveThread);
	delete m_shareMemoryClient;
	delete m_shareMemoryServer;
	CloseHandle(m_eventClientRead);
	CloseHandle(m_eventServerRead);
	DeleteCriticalSection(&m_dataCs);
	DeleteCriticalSection(&m_sendCs);
}

CHANNEL_ERROR MemoryChannel::InitChannel()
{
	CHANNEL_ERROR errorCode = CHANNEL_ERROR::NOT_ERROR;
	m_shareMemoryClient = new ShareMemory("Global\\SHARE_" + m_channelName + "_client");
	m_shareMemoryServer = new ShareMemory("Global\\SHARE_" + m_channelName + "_server");
	if (m_shareMemoryClient != NULL && m_shareMemoryServer != NULL)
	{
		m_shareMemoryClientAddr = m_shareMemoryClient->OpenShareMem(NULL, m_shareMemorySize);
		m_shareMemoryServerAddr = m_shareMemoryServer->OpenShareMem(NULL, m_shareMemorySize);
		if (m_shareMemoryClientAddr != NULL && m_shareMemoryServerAddr != NULL)
		{
			std::string clientReadEventName = "EVENT_" + m_channelName + "_client";
			std::string serverReadEventName = "EVENT_" + m_channelName + "_server";

			//事件都设置为手动恢复
			m_eventClientRead = CreateEventA(NULL, TRUE, FALSE, clientReadEventName.c_str());
			m_eventServerRead = CreateEventA(NULL, TRUE, FALSE, serverReadEventName.c_str());

			if (m_eventClientRead != NULL && m_eventServerRead != NULL)
			{
				//创建消息收发线程
				m_threadWorking = true;
				m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendDataThread, (LPVOID)this, 0, NULL);
				m_hReceiveThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveDataThread, (LPVOID)this, 0, NULL);
				errorCode = CHANNEL_ERROR::NOT_ERROR;
			}
			else
			{
				errorCode = CHANNEL_ERROR::EVENT_ERROR;
			}
		}
		else
		{
			errorCode = CHANNEL_ERROR::SHAREADDR_ERROR;
		}
	}
	else
	{
		errorCode = CHANNEL_ERROR::SHAREMEM_ERROR;
	}

	return errorCode;
}

void MemoryChannel::StoreSendData(std::string data)
{
	ubase::MyCriticalSection cs(&m_sendCs);
	if (m_sendDatas.size() < m_sendMaxSize)//如果队列中数据长度小于最大长度,则直接插入
	{
		m_sendDatas.push(data);
	}
	else//如果队列长度大于等于最大长度,则淘汰最早插入的一条数据,并插入最新的一条数据
	{
		m_sendDatas.push(data);
		m_sendDatas.pop();
	}
}

bool MemoryChannel::GetSendData(std::string& data)
{
	ubase::MyCriticalSection cs(&m_sendCs);
	bool result = false;
	if (m_sendDatas.size() != 0)
	{
		data = m_sendDatas.front();
		m_sendDatas.pop();
		result = true;
	}
	return result;
}

void MemoryChannel::StoreReceiveData(std::string data)
{
	ubase::MyCriticalSection cs(&m_dataCs);
	if (m_receiveDatas.size() < m_receiveMaxSize)//如果队列中数据长度小于最大长度,则直接插入
	{
		m_receiveDatas.push(data);
	}
	else//如果队列长度大于等于最大长度,则淘汰最早插入的一条数据,并插入最新的一条数据
	{
		m_receiveDatas.push(data);
		m_receiveDatas.pop();
	}
}

bool MemoryChannel::GetReceiveData(std::vector<std::string>& dataSet)
{
	ubase::MyCriticalSection cs(&m_dataCs);
	bool result = false;
	if (m_receiveDatas.size() != 0)
	{
		while (m_receiveDatas.empty() == false)
		{
			dataSet.push_back(m_receiveDatas.front());
			m_receiveDatas.pop();
		}

		if (dataSet.size() != 0)
		{
			result = true;
		}
	}
	return result;
}

unsigned  __stdcall MemoryChannel::SendDataThread(LPVOID args)
{
	MemoryChannel *p = (MemoryChannel*)args;
	HANDLE hEventRead = NULL;
	ShareMemory *pShareMem = NULL;
	void* pShareMemAddr = NULL;
	if (p->m_isServer == true) //如果当前为服务器端,则向客户端共享内存发送数据
	{
		hEventRead = p->m_eventClientRead;
		pShareMem = p->m_shareMemoryClient;
		pShareMemAddr = p->m_shareMemoryClientAddr;
	}
	else //如果当前为客户端,则向服务端共享内存发送数据
	{
		hEventRead = p->m_eventServerRead;
		pShareMem = p->m_shareMemoryServer;
		pShareMemAddr = p->m_shareMemoryServerAddr;
	}

	while (true)
	{
		if (WaitForSingleObject(hEventRead, 0) == WAIT_TIMEOUT) //如果等待事件超时,说明读取事件没有被激活,说明可以发送数据
		{
			std::string sendData;
			if (p -> GetSendData(sendData) == true)
			{
				int writeSize = pShareMem->WriteShareMem(pShareMemAddr, (void*)sendData.c_str(), sendData.length());
				if (writeSize == sendData.length())
				{
					SetEvent(hEventRead);//通知数据接收端进行读取数据
				}
			}
		}
		else //等待事件成功,说明读取事件被激活,说明现在共享内存处于繁忙状态,不能够写入
		{
			Sleep(10);
		}

		//如果发现线程需要停止,则退出循环体,停止线程
		if (p->m_threadWorking == false)
		{
			break;
		}
	}

	return 0;
}

unsigned  __stdcall MemoryChannel::ReceiveDataThread(LPVOID args)
{
	MemoryChannel *p = (MemoryChannel*)args;
	HANDLE hEventRead = NULL;
	ShareMemory *pShareMem = NULL;
	void* pShareMemAddr = NULL;
	if (p->m_isServer == true) //如果当前为服务器端,则从服务端共享内存获取数据
	{
		hEventRead = p->m_eventServerRead;
		pShareMem = p->m_shareMemoryServer;
		pShareMemAddr = p->m_shareMemoryServerAddr;
	}
	else //如果当前为客户端,则从客户端共享内存获取数据
	{
		hEventRead = p->m_eventClientRead;
		pShareMem = p->m_shareMemoryClient;
		pShareMemAddr = p->m_shareMemoryClientAddr;
	}

	while (true)
	{
		if (WaitForSingleObject(hEventRead, 500) == WAIT_OBJECT_0)
		{
			std::string receiveData;
			receiveData.resize(p->m_shareMemorySize);
			pShareMem->ReadShareMem(pShareMemAddr, (void*)receiveData.c_str(), p->m_shareMemorySize);

			if (p->m_pRecvDataFunc != NULL && p->m_pContext != NULL)//如果设置了回调函数接收数据,则优先使用回调函数传出数据
			{
				p->m_pRecvDataFunc(p->m_channelName.c_str(), receiveData.c_str(), receiveData.length(), p->m_pContext);//将接收到的数据通过回调函数传出
			}
			else
			{
				p->StoreReceiveData(receiveData);
			}
			
#ifdef _DEBUG
			printf("%s\n", receiveData.c_str());
#endif // _DEBUG
			ResetEvent(hEventRead);//数据存储已经完成,通知写入端可以写入了
		}

		//如果发现线程需要停止,则退出循环体,停止线程
		if (p->m_threadWorking == false)
		{
			break;
		}
	}

	return 0;
}
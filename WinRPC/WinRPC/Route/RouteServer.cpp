#include "RouteServer.h"
#include <json/json.h>

RouteServer::RouteServer(std::string routeManagerName, std::string serverName, unsigned channelMemSize, unsigned sendDatasMax, unsigned receiveDatasMax)
{
	m_routeManagerName = routeManagerName;
	m_serverName = serverName;
	m_channelMemSize = channelMemSize;
	m_sendDatasMax = sendDatasMax;
	m_receiveDatasMax = receiveDatasMax;
}

RouteServer::~RouteServer()
{
	if (m_hServerNoticeEvent != NULL)
	{
		CloseHandle(m_hServerNoticeEvent);
	}

	if (m_hServerMutex != NULL)
	{
		CloseHandle(m_hServerMutex);
	}

	if (m_serverTableMem != NULL)
	{
		delete m_serverTableMem;
	}
}

bool RouteServer::InitRouteManager()
{
	bool result = false;

	//Server相关的初始化
	std::string routeServerNoticeEventName = "ROUTE_SERVER_NOTICE_" + m_routeManagerName + "_WINRPC";
	m_hServerNoticeEvent = CreateEventA(NULL, FALSE, FALSE, routeServerNoticeEventName.c_str());
	std::string routeServerMutexName = "ROUTE_SERVER_MUTEX_" + m_routeManagerName + "_WINRPC";
	m_hServerMutex = CreateMutexA(NULL, TRUE, routeServerMutexName.c_str());
	std::string routeServerMemName = "Global\\ROUTE_SERVER_SHARE_" + m_routeManagerName + "_WINRPC";
	m_serverTableMem = new ShareMemory(routeServerMemName);

	if (m_serverTableMem != NULL)
	{
		m_serverMemAddr = m_serverTableMem->OpenShareMem(NULL, 4096);
		if (m_serverMemAddr != NULL)
		{
			result = AddServerRoute(m_serverName);
			if (result == true)//如果更新服务器表成功,通知所有客户端,也及时更新服务器表
			{
				SetEvent(m_hServerNoticeEvent);
			}
		}
	}
	return result;
}

/*
	共享内存中的服务器信息结构:
	{
		servers:[
			{"server_name":"hkxiaoyu118", "pid":2462},
			{"server_name":"zhoujielun", "pid":32452}
		]
	}
	server_name:服务器名称
	pid:服务器对应的进程ID
*/
bool RouteServer::AddServerRoute(std::string serverName)
{
	bool result = false;
	if (WaitForSingleObject(m_hServerMutex, INFINITE) == WAIT_OBJECT_0)
	{
		std::string oldRouteData;	//旧的服务器信息
		std::string newRouteData;	//新的服务器信息
		oldRouteData.resize(4096);
		m_serverTableMem->ReadShareMem(m_serverMemAddr, (void*)oldRouteData.c_str(), 4096);
		if (oldRouteData.empty() == false)//如果存储服务器信息的共享内存中有数据,则先读取原有的数据
		{
			Json::Value rootValue;
			Json::Reader reader;
			if (reader.parse(oldRouteData, rootValue) == true)
			{
				if (rootValue["servers"].isArray())
				{
					Json::Value routeValue;
					Json::FastWriter writer;
					routeValue["server_name"] = serverName;
					routeValue["pid"] = (unsigned)GetCurrentProcessId();
					rootValue["servers"].append(routeValue);
					newRouteData = writer.write(rootValue);
				}
			}
		}
		else//如果存储服务器信息的共享内存中没有数据,则直接填写数据 
		{
			Json::Value rootValue;
			Json::Value routeValue;
			Json::FastWriter writer;
			routeValue["server_name"] = serverName;
			routeValue["pid"] = (unsigned)GetCurrentProcessId();
			rootValue["servers"].append(routeValue);
			newRouteData = writer.write(rootValue);
		}

		if (newRouteData.empty() == false)
		{
			m_serverTableMem->WriteShareMem(m_serverMemAddr, (void*)newRouteData.c_str(), 4096);
			result = true;
		}
		
		ReleaseMutex(m_hServerMutex);
	}
	return result;
}

bool RouteServer::DelServer(std::string serverName)
{
	bool result = false;
	return result;
}

bool RouteServer::SendData(std::string clientName, std::string data)
{
	m_sendDatasMutex.lock();
	MsgNode msg;
	msg.clientName = clientName;
	msg.data = data;
	m_sendDatas.push(msg);
	m_sendDatasMutex.unlock();
	return true;
}

bool RouteServer::GetSendData(MsgNode& data)
{
	bool result = false;
	m_sendDatasMutex.lock();
	if (m_sendDatas.size() != 0)
	{
		data = m_sendDatas.front();
		m_sendDatas.pop();
		result = true;
	}
	m_sendDatasMutex.unlock();
	return result;
}

bool RouteServer::StoreReceivedData(std::string clientName, std::string data)
{
	MsgNode msg;
	msg.clientName = clientName;
	msg.data = data;
	m_receiveDatasMutex.lock();
	m_receiveDatas.push(msg);
	m_receiveDatasMutex.unlock();
	return true;
}

bool RouteServer::GetReceivedData(std::vector<MsgNode>& data)
{
	bool result = false;
	m_receiveDatasMutex.lock();
	while (m_receiveDatas.empty() == false)
	{
		data.push_back(m_receiveDatas.front());
		m_receiveDatas.pop();
	}
	m_receiveDatasMutex.unlock();
	return result;
}

bool RouteServer::AddClient(std::string clientName)
{
	bool result = false;
	const std::string channelName = m_serverName + "_" + clientName;
	//这里建立通道使用了(服务器名称+"_"+客户端名称),为了能够再添加了新的服务器以后,让客户端能够区分
	MemoryChannel* pChannel = new MemoryChannel(channelName, true, m_channelMemSize, m_sendDatasMax, m_receiveDatasMax);
	if (pChannel != NULL)
	{
		if (pChannel->InitChannel() == NOT_ERROR)
		{
			m_clientChannelsMutex.lock();
			m_clientChannels[clientName] = pChannel;//添加客户端通道
			m_clientChannelsMutex.unlock();
			result = true;
		}
	}
	return result;
}
bool RouteServer::DelClient(std::string clientName)
{
	bool result = false;
	m_clientChannelsMutex.lock();
	if (m_clientChannels.find(clientName) != m_clientChannels.end())
	{
		MemoryChannel* pChannel = m_clientChannels[clientName];
		delete pChannel;
		m_clientChannels.erase(clientName);//删除指定名称的客户端
		result = true;
	}
	m_clientChannelsMutex.unlock();
	return result;
}
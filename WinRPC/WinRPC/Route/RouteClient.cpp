#include "RouteClient.h"

RouteClient::RouteClient(std::string routeManagerName, unsigned channelMemSize, unsigned sendDatasMax, unsigned receiveDatasMax)
{
	m_routeManagerName = routeManagerName;
	m_channelMemSize = channelMemSize;
	m_sendDatasMax = sendDatasMax;
	m_receiveDatasMax = receiveDatasMax;
}

RouteClient::~RouteClient()
{

}

bool RouteClient::InitRouteClient()
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
			//获取共享内存中的服务器列表
			std::string serverData;	//共享内存中存储的服务器信息
			serverData.resize(4096);
			if (WaitForSingleObject(m_hServerMutex, INFINITE) == WAIT_OBJECT_0)
			{
				m_serverTableMem->ReadShareMem(m_serverMemAddr, (void*)serverData.c_str(), 4096);
				ReleaseMutex(m_hServerMutex);//读取完共享内存以后,释放锁

				if (serverData.empty() == false)//如果存储服务器信息的共享内存中有数据,则先读取原有的数据
				{
					Json::Value rootValue;
					Json::Reader reader;
					if (reader.parse(serverData, rootValue) == true)
					{
						if (rootValue["servers"].isArray())
						{
							for (int i = 0; i < rootValue["servers"].size(); i++)
							{
								Json::Value serverValue = rootValue["servers"][i];
								ServerNode serverNode;
								serverNode.serverName = serverValue["server_name"].asString();
								serverNode.pid = serverValue["pid"].asUInt();
								m_serverNodes[serverNode.serverName] = serverNode;
							}

							if (m_serverNodes.size() != 0)
							{
								for (auto iter = m_serverNodes.begin(); iter != m_serverNodes.end(); iter++)
								{
									std::string serverName = iter->first;
									AddServer(serverName);
								}
							}

							result = true;
						}
					}
				}
			}
		}
	}
	return result;
}

bool RouteClient::AddServer(std::string serverName)
{
	bool result = false;
	const std::string channelName = serverName + "_" + m_clientName;
	//这里建立通道使用了(服务器名称+"_"+客户端名称),为了能够再添加了新的服务器以后,让客户端能够区分
	MemoryChannel* pChannel = new MemoryChannel(channelName, false , m_channelMemSize, m_sendDatasMax, m_receiveDatasMax);
	if (pChannel != NULL)
	{
		if (pChannel->InitChannel() == NOT_ERROR)
		{
			m_serverChannelsMutex.lock();
			m_serverChannels[serverName] = pChannel;//添加客户端通道
			m_serverChannelsMutex.unlock();
			result = true;
		}
	}
	return result;
}

void RouteClient::BroadcastData(std::string data)
{
	m_serverChannelsMutex.lock();
	// 向所有的服务器发送一份数据(向服务器广播数据)
	for (auto iter = m_serverChannels.begin(); iter != m_serverChannels.end(); iter++)
	{
		MemoryChannel* pChannel = iter->second;
		pChannel->StoreSendData(data);
	}
	m_serverChannelsMutex.unlock();
}

void RouteClient::SendData(std::string serverName, std::string data)
{
	m_serverChannelsMutex.lock();
	//向指定的服务器发送数据(定向发送)
	if (m_serverChannels.find(serverName) != m_serverChannels.end())
	{
		MemoryChannel* pChannel = m_serverChannels[serverName];
		if (pChannel != NULL)
		{
			pChannel->StoreSendData(data);
		}
	}
	m_serverChannelsMutex.unlock();
}

bool GetReceivedDatas(std::vector<MsgNode>& datas)
{
	bool result = false;
	return result;
}
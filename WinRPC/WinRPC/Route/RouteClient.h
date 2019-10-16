#pragma once
#include <string>
#include <queue>
#include <vector>
#include <map>
#include <mutex>
#include <process.h>
#include "../Common/Utility.h"
#include "../Common/ShareMemory.h"
#include "../Channel/MemoryChannel.h"
#include "RouteUtility.h"

class RouteClient
{
public:
	RouteClient(std::string routeManagerName, std::string clientName, unsigned channelMemSize = 1024 * 4, unsigned sendDatasMax = 100, unsigned receiveDatasMax = 100);
	~RouteClient();
	bool InitRouteClient();
	bool AddServer(std::string serverName);	//添加服务端
	bool IsServerExist(std::string serverName);//判断服务端是否已经存在
	void BroadcastData(std::string data);	//向服务区发送数据(广播方式)
	void SendData(std::string serverName, std::string data);//向服务器发送数据(定向发送)
	void GetReceivedDatas(std::vector<MsgNode>& datas);//获取接收的数据(所有)
	void StoreReceivedData(std::string serverName, std::string data);//存储从服务器收到的数据
	static unsigned __stdcall ServerInfoMonitorThread(LPVOID args);//监视服务器信息的线程
	static void __stdcall RecvDataCallback(const char* channelName, const char* data, unsigned int dataLength, void* pContext);//接收数据的回调函数
private:
	std::string m_routeManagerName;	//路由管理器的名称
	std::string m_clientName;		//本客户端的名称
	unsigned int m_channelMemSize;	//通道使用的共享内存大小
	unsigned int m_sendDatasMax;	//发送队列的最大长度
	unsigned int m_receiveDatasMax;	//接收队列的最大长度

	std::queue<MsgNode> m_receiveDatasQueue;	//接收消息队列
	std::mutex m_receiveDatasMutex;				//接收消息队列锁

	HANDLE m_hServerNoticeEvent;
	HANDLE m_hServerMutex;
	ShareMemory* m_serverTableMem;
	void* m_serverMemAddr;

	std::map<std::string, MemoryChannel*> m_serverChannels;	//服务器通道
	std::mutex m_serverChannelsMutex;						//服务器通道锁
	HANDLE m_hServerInfoMonitorThread;						//服务器信息监视线程句柄
	bool m_serverInfoMonitorThRunning;						//服务器信息监视线程是否运行
};

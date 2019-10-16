#pragma once
#include "../Common/ShareMemory.h"
#include "../Channel/MemoryChannel.h"
#include "../Common/Utility.h"
#include "RouteUtility.h"
#include <string>
#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <process.h>
#include <windows.h>

class RouteServer
{
public:
	RouteServer(std::string routeManagerName, std::string serverName, unsigned channelMemSize = 1024 * 4, unsigned sendDatasMax = 100, unsigned receiveDatasMax = 100);
	~RouteServer();
	bool InitRouteManager();											//初始化路由管理器
	bool AddServerRoute(std::string serverName);						//添加服务器
	bool DelServer(std::string serverName);								//删除服务器
	bool SendData(std::string clientName, std::string data);			//向客户端发送数据(如果clientName为空,则向所有的客户端发送数据)
	bool GetSendData(std::queue<MsgNode>& data);						//从队列中,一次性取出所有将要发送的数据
	void StoreReceivedData(std::string clientName, std::string data);	//存储从客户端收到的数据
	void GetReceivedData(std::vector<MsgNode>& data);					//获取客户端发来的所有数据
	bool AddClient(std::string clientName);								//添加一个新的客户端
	bool DelClient(std::string clientName);								//删除一个客户端
	void SendDataToChannel(const MsgNode& msgNode);						//向管道发送数据

	static void __stdcall RecvDataCallback(const char* channelName, const char* data, unsigned int dataLength, void* pContext);//接收数据的回调函数
	static unsigned __stdcall SendDataThread(LPVOID args);				//负责发送数据的线程
private:
	std::string m_routeManagerName;	//路由管理器的名称
	std::string m_serverName;		//此服务端的名称

	//Server相关
	HANDLE m_hServerNoticeEvent;	//Server更新事件
	ShareMemory* m_serverTableMem;	//存放Server表信息的共享内存
	void* m_serverMemAddr;			//存放Server信息共享内存的内存映射地址
	HANDLE m_hServerMutex;			//Server路由共享内存访问互斥锁(全局)

	//收发数据相关
	std::queue<MsgNode> m_receiveDatas;	//接收从Client发送过来数据的队列
	std::mutex m_receiveDatasMutex;		//接收数据队列操作锁
	unsigned int m_receiveDatasMax;		//接收队列缓冲最大长度(如果超过将淘汰最早的)
	std::queue<MsgNode> m_sendDatas;	//向Client发送数据的队列
	std::mutex m_sendDatasMutex;		//发送数据队列操作锁
	unsigned int m_sendDatasMax;		//发送队列缓冲区最大长度(如果超过将淘汰最早的)
	HANDLE m_hSendDataThread;			//数据发送线程句柄
	bool m_sendDataThreadRunning;		//数据发送线程运行控制

	//通信通道相关
	std::map<std::string, MemoryChannel*> m_clientChannels;//客户端频道
	std::mutex m_clientChannelsMutex;	//客户端频道锁
	unsigned int m_channelMemSize;		//创建通信管道的共享内存大小
};

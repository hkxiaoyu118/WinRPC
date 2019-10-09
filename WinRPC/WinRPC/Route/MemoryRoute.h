#pragma once
#include <string>
#include <map>
#include <queue>
#include "../Common/ShareMemory.h"
#include "../Common/MyCriticalSection.h"


class MemoryRoute
{
public:
	MemoryRoute();
	~MemoryRoute();

	bool InitRoute(); //路由初始化
	static unsigned __stdcall SendDataThread(LPVOID args);		//发送数据线程
	static unsigned __stdcall ReceiveDataThread(LPVOID args);	//接收数据线程
private:
	std::map<std::string, ShareMemory*> m_topicMaps;
	std::map<std::string, HANDLE> m_eventHandleMaps;
	HANDLE m_hSendDataThread;
	HANDLE m_hReceiveDataThread;
};

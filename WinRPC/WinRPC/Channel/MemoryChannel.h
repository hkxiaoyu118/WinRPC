#pragma once
#include <string>
#include <vector>
#include <queue>
#include <windows.h>
#include "../Common/ShareMemory.h"
#include "../Common/MyCriticalSection.h"

/*
	内存channel
*/

enum CHANNEL_ERROR
{
	NOT_ERROR,
	SHAREMEM_ERROR,
	SHAREADDR_ERROR,
	EVENT_ERROR
};

class MemoryChannel
{
public:
	MemoryChannel(const std::string channelName, bool isServer = false, DWORD shareMemorySize = 1024 * 4, unsigned sendMaxSize = 100, unsigned receiveMaxSize = 100);
	~MemoryChannel();
	CHANNEL_ERROR InitChannel();
	void StoreSendData(std::string data);
	bool GetSendData(std::string& data);
	void StoreReceiveData(std::string data); 
	bool GetReceiveData(std::queue<std::string>& dataSet);
	static unsigned  __stdcall SendDataThread(LPVOID args);
	static unsigned  __stdcall ReceiveDataThread(LPVOID args);


private:
	std::string m_channelName;			//通道名称
	ShareMemory *m_shareMemoryClient;	//客户端共享内存
	ShareMemory *m_shareMemoryServer;	//服务端共享内存
	void* m_shareMemoryClientAddr;		//客户端共享内存地址
	void* m_shareMemoryServerAddr;		//服务器共享内存地址
	HANDLE m_eventClientRead;			//客户端读事件
	HANDLE m_eventServerRead;			//服务端读事件
	DWORD m_shareMemorySize;			//共享内存的大小(单位字节)
	bool m_isServer;					//调用者是否是服务端

	std::queue<std::string> m_receiveDatas;
	CRITICAL_SECTION m_dataCs;

	std::queue<std::string> m_sendDatas;
	CRITICAL_SECTION m_sendCs;

	unsigned m_sendMaxSize;
	unsigned m_receiveMaxSize;

	bool m_threadWorking;				//判断线程是否工作
	HANDLE m_hSendThread;				//发消息线程句柄
	HANDLE m_hReceiveThread;			//收消息线程句柄
};

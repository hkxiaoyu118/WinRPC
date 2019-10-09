#pragma once
#include <map>
#include <queue>
#include <string>
#include "../Channel/MemoryChannel.h"

class ChannelManager
{
public:
	ChannelManager();
	~ChannelManager();
	bool AddChannel(
		const std::string channelName,		//通道名称
		bool isServer = false,				//是否是服务端,默认是客户端
		DWORD shareMemorySize = 1024 * 4,   //共享内存的大小,默认是4K
		unsigned sendMaxSize = 100,			//发送数据的最大存储条数,默认是100条
		unsigned receiveMaxSize = 100		//接收数据的最大存储条数,默认是100条
		);//添加通道

	bool IsChannelExist(const std::string channelName);//判断通道是否已经存在
	void AddChannelItem(const std::string channelName, MemoryChannel* pChannel);//向map中添加通道
	void DelChannelItem(const std::string channelName);//从map删除通道
	bool SendData(const std::string channelName, std::string data);//向指定的通道发送数据
	bool GetData(const std::string channelName, std::queue<std::string>& data);//从指定的通道中取数据(所有)
private:
	std::map<std::string, MemoryChannel*> m_channelMaps;
	CRITICAL_SECTION m_channelMapsCS;
};

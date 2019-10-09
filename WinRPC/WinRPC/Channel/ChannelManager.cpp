#include "ChannelManager.h"
#include "../Common/MyCriticalSection.h"

ChannelManager::ChannelManager()
{
	InitializeCriticalSection(&m_channelMapsCS);
}

ChannelManager::~ChannelManager()
{
	DeleteCriticalSection(&m_channelMapsCS);
}

bool ChannelManager::AddChannel(const std::string channelName, bool isServer, DWORD shareMemorySize, unsigned sendMaxSize, unsigned receiveMaxSize)
{
	bool result = false;
	if (channelName.empty() == false)
	{
		MemoryChannel* pChannel = new MemoryChannel(channelName, isServer, shareMemorySize, sendMaxSize, receiveMaxSize);
		if (pChannel != NULL && IsChannelExist(channelName) == false)
		{
			CHANNEL_ERROR initResult = pChannel->InitChannel();
			if (initResult == CHANNEL_ERROR::NOT_ERROR)
			{
				AddChannelItem(channelName, pChannel);
				result = true;
			}
		}
	}
	return result;
}

bool ChannelManager::IsChannelExist(const std::string channelName)
{
	ubase::MyCriticalSection cs(&m_channelMapsCS);
	bool result = false;
	if (m_channelMaps.find(channelName) != m_channelMaps.end())
	{
		result = true;
	}
	return result;
}

void ChannelManager::AddChannelItem(const std::string channelName, MemoryChannel* pChannel)
{
	ubase::MyCriticalSection cs(&m_channelMapsCS);
	m_channelMaps[channelName] = pChannel;
}

void ChannelManager::DelChannelItem(const std::string channelName)
{
	ubase::MyCriticalSection cs(&m_channelMapsCS);
	auto iter = m_channelMaps.find(channelName);
	if (iter != m_channelMaps.end())
	{
		MemoryChannel* pChannel = iter->second;
		delete pChannel;
		m_channelMaps.erase(iter);
	}
}

bool ChannelManager::SendData(const std::string channelName, std::string data)
{
	ubase::MyCriticalSection cs(&m_channelMapsCS);
	MemoryChannel* pChannel = m_channelMaps[channelName];
	if (pChannel != NULL)
	{
		pChannel->StoreSendData(data);
		return true;
	}
	return false;
}

bool ChannelManager::GetReceiveData(const std::string channelName, std::vector<std::string>& data)
{
	ubase::MyCriticalSection cs(&m_channelMapsCS);
	bool result = false;
	MemoryChannel* pChannel = m_channelMaps[channelName];
	if (pChannel != NULL)
	{
		result = pChannel->GetReceiveData(data);
	}
	return result;
}
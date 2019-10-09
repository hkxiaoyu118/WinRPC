#include "MemoryRoute.h"
#include <process.h>

MemoryRoute::MemoryRoute()
{

}

MemoryRoute::~MemoryRoute()
{

}

bool MemoryRoute::InitRoute()
{
	bool result = false;
	m_hSendDataThread = (HANDLE)_beginthreadex(NULL, 0, SendDataThread, this, 0, NULL);
	m_hReceiveDataThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveDataThread, this, 0, NULL);
	if (m_hSendDataThread != NULL && m_hReceiveDataThread != NULL)
	{

	}
	return result;
}

unsigned __stdcall MemoryRoute::SendDataThread(LPVOID args)
{
	return 0;
}

unsigned __stdcall MemoryRoute::ReceiveDataThread(LPVOID args)
{
	return 0;
}

#pragma once
#include <vector>
#include <string>
#include <windows.h>

typedef void __stdcall FunProcessRecvData(const char* channelName, const char* data, unsigned int dataLength, void* pContext);

//服务端的详细信息
struct ClientNode
{
	std::string clientName;	//客户端的名称
	DWORD pid;				//客户端所在进程的PID
};

struct MsgNode
{
	std::string clientOrServerName;	//客户端的名称
	std::string data;				//数据的内容
};

struct ServerNode
{
	std::string serverName;	//服务端的名称
	DWORD pid;				//服务端所在进程的PID
};

std::vector<std::string> StrSplit(std::string str, std::string pattern);
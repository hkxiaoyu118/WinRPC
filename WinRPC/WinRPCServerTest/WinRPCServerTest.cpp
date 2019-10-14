// WinRPCServerTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "../WinRPC/Channel/ChannelManager.h"
#include "../WinRPC/Route/RouteServer.h"
#pragma comment(lib, "../Debug/WinRPC.lib")

void ChannelManagerTest()
{
	ChannelManager manager;
	const std::string channelName = "hkxiaoyu118";
	if (manager.AddChannel(channelName, true) == true)
	{
		std::cout << "添加通道成功" << std::endl;
	}
	else
	{
		std::cout << "添加通道失败" << std::endl;
	}

	unsigned int count = 0;
	while (true)
	{
		std::string data = "this is a server msg:" + std::to_string(count);
		manager.SendData(channelName, data);
		count++;
		Sleep(20);
	}
	system("pause");
}

void RouteServerTest()
{
	std::string routeName = "hkxiaoyu118";
	std::string serverName = "hkxiaoyu";
	RouteServer routeServer(routeName, serverName);
	if (routeServer.InitRouteManager() == true)
	{
		routeServer.AddClient("zhoujielun");
		std::cout << "InitRouteManager Success" << std::endl;
	}
	else
	{
		std::cout << "InitRouteManager Error" << std::endl;
	}

	unsigned int count = 0;
	while (true)
	{
		std::string data = "this is a server msg:" + std::to_string(count);
		routeServer.SendData("zhoujielun", data);
		count++;
		Sleep(20);
	}

	system("pause");
}

int _tmain(int argc, _TCHAR* argv[])
{
	RouteServerTest();
	return 0;
}

// WinRPCClientTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../WinRPC/Channel/ChannelManager.h"
#include "../WinRPC/Route/RouteClient.h"
#pragma comment(lib, "../Debug/WinRPC.lib")

void ChannelManagerTest()
{
	ChannelManager manager;
	const std::string channelName = "hkxiaoyu118";
	if (manager.AddChannel(channelName, false) == true)
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
		std::string data = "this is a client msg:" + std::to_string(count);
		manager.SendData(channelName, data);
		count++;
		Sleep(20);
	}

	//	manager.DelChannelItem(channelName);
	system("pause");
}

void RouteClientTest()
{
	std::string routeName = "hkxiaoyu118";
	std::string clientName = "zhoujielun";
	RouteClient routeClient(routeName, clientName);
	if (routeClient.InitRouteClient() == true)
	{
		std::cout<< "InitRouteClient Success" << std::endl;
	}
	else
	{
		std::cout << "InitRouteClient Error" << std::endl;
	}

	unsigned int count = 0;
	while (true)
	{
		std::string data = "this is a client msg:" + std::to_string(count);
		//routeClient.SendData("hkxiaoyu", data);
		routeClient.BroadcastData(data);
		count++;
		Sleep(20);
	}
	system("pause");
}

int _tmain(int argc, _TCHAR* argv[])
{
	RouteClientTest();
	return 0;
}


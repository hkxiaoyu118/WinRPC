#pragma once
#include "../Common/ShareMemory.h"
#include "MemoryRoute.h"
#include <string>
#include <map>
#include <mutex>
#include <windows.h>

//路由器的详细信息
struct RouteNode
{
	std::string routeName;	//路由器的名称
	DWORD pid;				//路由器所在进程的PID
};

class RouteServer
{
public:
	RouteServer();
	~RouteServer();
	bool InitRouteManager();				//初始化路由管理器
	bool AddRoute(std::string routeName);	//向路由管理器中,添加路由器
	bool DelRoute(std::string routeName);	//从路由管理器中,删除路由器
	bool UpdateLocalRouteTable();			//更新本地保存的路由表
	static unsigned __stdcall MonitorRouteTableUpdateThread(LPVOID args);//监视路由表信息的线程
private:
	std::string m_routeManagerName;		//路由管理器名称
	HANDLE m_hRouteUpdateNoticeEvent;	//路由更新事件
	ShareMemory* m_routeUpdateMem;		//存放路由信息的共享内存
	void* m_routeUpdateMemAddr;			//存放路由信息共享内存的内存映射地址
	HANDLE m_hRouteMutex;				//路由共享内存访问互斥锁(全局)
	MemoryRoute* m_memRoute;			//内存路由
	std::string m_routeName;			//此路由器的名称

	std::map<std::string, RouteNode> m_routeMaps;	//路由表的内容
	std::mutex m_routeMapsMutex;					//路由表内容控制锁
};

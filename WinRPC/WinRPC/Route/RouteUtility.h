#pragma once
#include "../Common/Utility.h"
#include "../TinyXML2/tinyxml2.h"
#include <queue>
#include <string>
#include <windows.h>

bool ReadShareMemServerInfo(std::string shareMemStr, std::vector<ServerNode>& serverNodes); //读取共享内存中的服务器信息
std::string CreateShareMemServerInfo(std::vector<ServerNode> serverNodes);					//设置共享内存中的服务器信息
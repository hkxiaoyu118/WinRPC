#include "RouteUtility.h"

/*
	//共享内存中的服务器信息结构(XML格式):
	//xml格式
	<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>
	<servers>
		<server name="hkxiaoyu118" pid=2462 />
		<server name="zhoujielun" pid=32452 />
		<server name="zhangxiongmao" pid=12312 />
	</userinfo>

	name:服务器名称
	pid:服务端对应的进程ID
*/
bool ReadShareMemServerInfo(std::string shareMemStr, std::vector<ServerNode>& serverNodes)
{
	bool result = false;
	if (shareMemStr.empty() == false)
	{
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError errorCode = doc.Parse(shareMemStr.c_str());
		if (tinyxml2::XML_SUCCESS == errorCode)
		{
			tinyxml2::XMLElement* serversElement = doc.FirstChildElement("servers");
			if (serversElement != NULL)
			{
				tinyxml2::XMLElement* serverElement = NULL;
				for (serverElement = serversElement->FirstChildElement(); serverElement != NULL; serverElement = serverElement->NextSiblingElement())
				{
					const char* szServerName = serverElement->Attribute("name");
					unsigned int pid = serverElement->UnsignedAttribute("pid");
					if (szServerName != NULL && pid != 0)
					{
						ServerNode serverNode;
						serverNode.serverName = szServerName;
						serverNode.pid = pid;
						serverNodes.push_back(serverNode);
					}
				}
			}
		}
	}

	if (serverNodes.size() != 0)
	{
		result = true;
	}

	return result;
}

std::string CreateShareMemServerInfo(std::vector<ServerNode> serverNodes)
{
	std::string serverInfoStr;
	const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError errorCode = doc.Parse(declaration);
	if (tinyxml2::XML_SUCCESS == errorCode)
	{
		tinyxml2::XMLElement* serversElement = doc.NewElement("servers");
		doc.InsertEndChild(serversElement);

		for (std::vector<ServerNode>::iterator iter = serverNodes.begin(); iter != serverNodes.end(); iter++)
		{
			ServerNode serverNode = *iter;
			tinyxml2::XMLElement* serverElement = doc.NewElement("server");
			serverElement->SetAttribute("name", serverNode.serverName.c_str());
			serverElement->SetAttribute("pid", (unsigned)serverNode.pid);
			serversElement->InsertEndChild(serverElement);
		}

		tinyxml2::XMLPrinter printer;
		doc.Print(&printer);
		serverInfoStr = printer.CStr();
	}
	return serverInfoStr;
}

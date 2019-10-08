#pragma once
#include <windows.h>
#include <string>
#include <iostream>

class ShareMemory
{
public:
	ShareMemory(const std::string shareMemName, bool createFile = false);
	~ShareMemory();

	void* OpenShareMem(void* addr, const unsigned length, DWORD protect);	//打开共享内存
	int WriteShareMem(void* dest, void*src, unsigned size);					//写共享内存
	int ReadShareMem(void* src, void*dest, unsigned size);					//读共享内存
private:
	bool CheckAddress(void* addr);	//检查内存地址是否合法
private:
	std::string m_shareMemName;		//共享内存的名字
	bool m_isCreateFile;			//文件共享内存or内存页共享内存
	void* m_shareMemAddress;		//共享内存的基地址
	HANDLE m_semaphore;				//信号
	HANDLE m_fileMapping;			//文件句柄
	unsigned int m_shareMemSize;	//共享内存的大小
};

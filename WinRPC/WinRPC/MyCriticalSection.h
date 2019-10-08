#pragma once
#include <windows.h>

namespace ubase
{
	/*
		临界区包装类(用于控制作用域范围的临界区)
	*/
	class MyCriticalSection
	{
	public:
		MyCriticalSection(CRITICAL_SECTION* criticalSection)
		{
			m_criticalSection = criticalSection;
			::EnterCriticalSection(m_criticalSection);//进入临界区
		}
		~MyCriticalSection()
		{
			::LeaveCriticalSection(m_criticalSection);//退出临界区
		}
	private:
		CRITICAL_SECTION* m_criticalSection;
	};

	/*
		临界区包装类(用于手动控制临界区)
	*/
	class CriticalSection
	{
	public:
		CriticalSection()
		{
			::InitializeCriticalSection(&m_criticalSection);
		}

		~CriticalSection()
		{
			::DeleteCriticalSection(&m_criticalSection);
		}

		void Lock()
		{
			::EnterCriticalSection(&m_criticalSection);
		}

		void UnLock()
		{
			::LeaveCriticalSection(&m_criticalSection);
		}
	private:
		CRITICAL_SECTION m_criticalSection;
	};
}

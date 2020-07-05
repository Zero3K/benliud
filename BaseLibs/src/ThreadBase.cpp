/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#include <stdio.h>

#if defined(WIN32)
#include <process.h> /* _beginthreadex, _endthreadex */
#include <windows.h>
#elif defined(_WIN32_WCE)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../include/ThreadBase.h"

namespace SockLib
{
	CThreadBase::CThreadBase()
	{

	}

	bool CThreadBase::Run(bool detached)
	{
		m_bDetached=detached;

#ifdef WIN32
		//if return is 0 then fail. the return value infact is a handle
		//if(0==(m_hThread=(void*)_beginthreadex(NULL, 0, &_Swap, this, 0, &m_ThreadId)))
		//{
		//	Sleep(300);
		//	m_hThread=(void*)_beginthreadex(NULL, 0, &_Swap, this, 0, &m_ThreadId);
		//	return 0!=m_hThread;
		//}

		m_hThread=::CreateThread(NULL, 0, &_Swap, this, 0, &m_ThreadId);
		return m_hThread!=NULL;
#else
		if(m_bDetached)
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			//if return !=0 then fail
			if(pthread_create(&m_ThreadId,&attr,_Swap,this)!=0)
			{
				usleep(30*1000);
				return 0!=pthread_create(&m_ThreadId,&attr,_Swap,this);
			}
		}
		else
		{
			//if return !=0 then fail
			if(pthread_create(&m_ThreadId,NULL,_Swap,this)!=0)
			{
				usleep(30*1000);
				return 0!=pthread_create(&m_ThreadId,NULL,_Swap,this);
			}
		}


#endif

		return true;

	}

	CThreadBase::~CThreadBase()
	{

	}


#ifdef WIN32
	DWORD WINAPI CThreadBase::_Swap(void* PTHIS)
#else
	void* CThreadBase::_Swap(void* PTHIS)
#endif
	{
#define PT ((CThreadBase*)PTHIS)

		PT -> Entry();

		if(PT->m_bDetached)	delete PT;  //a detached thread will delete itself when quit

#ifdef WIN32
		::ExitThread(0);
		return 0;
#else
		return (void*)NULL;
#endif


#undef PT
	}


	void CThreadBase::ReleaseCPU()
	{
#ifdef WIN32
		Sleep(0);
#else
		sched_yield();	
#endif	
	}

	void CThreadBase::Stop()
	{

	}

	//when run a joinable thread ,call it! 
	void CThreadBase::Wait()
	{
		if(m_bDetached) return ;// detached thread no need to wait
#ifdef WIN32
		::WaitForSingleObject(m_hThread,INFINITE);
		::CloseHandle(m_hThread);
#else
		pthread_join(m_ThreadId,NULL);
#endif
	}

	//only work for windows this time
	/*
	prio  range in windows:
	THREAD_PRIORITY_ABOVE_NORMAL
	THREAD_PRIORITY_BELOW_NORMAL
	THREAD_PRIORITY_HIGHEST
	THREAD_PRIORITY_IDLE
	THREAD_PRIORITY_LOWEST
	THREAD_PRIORITY_NORMAL
	THREAD_PRIORITY_TIME_CRITICAL
	This parameter can also be -7, -6, -5, -4, -3, 3, 4, 5, or 6.
	*/
	void CThreadBase::SetPriority(int prio)
	{
#ifdef WIN32
		SetThreadPriority(m_hThread,prio);
#else
		pthread_setschedprio(pthread_self(), prio);	//mac don't have this!
#endif
	}

}
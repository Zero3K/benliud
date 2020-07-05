
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include <assert.h>
#include "../include/Mutex.h"
namespace SockLib
{
	CMutex::CMutex()
	{

#ifdef WIN32
		m_mutex = ::CreateMutex(NULL, FALSE, NULL);
		assert(m_mutex);
#else
		//here make a change for bittorrent to use.
		//if conflict with other module,should change again.
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_mutex, &attr);
		pthread_mutexattr_destroy(&attr);
		//can't recursively use
		//	pthread_mutex_init(&m_mutex, NULL);
#endif
	}


	CMutex::~CMutex()
	{
#ifdef WIN32
		if(m_mutex)	::CloseHandle(m_mutex);
#else
		pthread_mutex_destroy(&m_mutex);
#endif
	}


	void CMutex::Lock()
	{
#ifdef WIN32

		if(m_mutex==NULL) m_mutex = ::CreateMutex(NULL, FALSE, NULL);

		assert(m_mutex);
		DWORD d = WaitForSingleObject(m_mutex, INFINITE);

#else
		pthread_mutex_lock(&m_mutex);
#endif
	}


	void CMutex::Unlock()
	{
#ifdef WIN32
		::ReleaseMutex(m_mutex);
#else
		pthread_mutex_unlock(&m_mutex);
#endif
	}

}


/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/
#ifndef _SL_MUTEX_H
#define _SL_MUTEX_H

#ifndef WIN32
#include <pthread.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

namespace SockLib
{
	class CMutex
	{
		friend class CLock;
		friend class CCondition;
	public:
		CMutex();
		~CMutex();

		void Lock();
		void Unlock();
	protected:
#ifdef WIN32
		HANDLE m_mutex;
#else
		pthread_mutex_t m_mutex;
#endif
	};


}

#endif // _SOCKETS_MUTEX_H

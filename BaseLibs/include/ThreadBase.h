/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _THREADBASE_H
#define _THREADBASE_H


#if defined(WIN32)||defined(WINCE)
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace SockLib
{
	class CThreadBase
	{
	public:
		void SetPriority(int prio);
		virtual void Wait();
		virtual void Stop();
		void ReleaseCPU();
		CThreadBase();
		virtual ~CThreadBase();

		virtual void Entry() = 0;
		//	void Yield(); //have compile error in vc6; changed to ReleaseCPU
		bool Run(bool detached=true);

	private:
#ifdef WIN32
		static DWORD WINAPI _Swap(void*);
#else
		static void* _Swap(void*);
#endif
		//CThreadBase(const CThreadBase& ) {}
		//CThreadBase& operator=(const CThreadBase& ) { return *this; }
	protected:
#ifdef WIN32
		void* m_hThread;  //infact it's a HANDLE
		DWORD m_ThreadId;
#else
		pthread_t m_ThreadId;
#endif

	private:
		bool m_bDetached;

	};

}

#endif // _SOCKETS_THREAD_H

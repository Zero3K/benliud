/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/
#include "../include/AutoLock.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace SockLib
{
	CAutoLock::CAutoLock(CMutex& mutex)
		:m_mutex(mutex)
	{
		m_mutex.Lock();
	}

	CAutoLock::~CAutoLock()
	{
		m_mutex.Unlock();
	}

}

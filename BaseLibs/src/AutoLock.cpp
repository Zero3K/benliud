/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

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

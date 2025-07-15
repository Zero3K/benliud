/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

// AutoLock.h: interface for the CAutoLock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AUTOLOCK_H
#define _AUTOLOCK_H

#include "Mutex.h"

namespace SockLib
{
	class CAutoLock  
	{
	public:
		CAutoLock(CMutex& mutex);
		virtual ~CAutoLock();
	protected:
		CMutex& m_mutex;
	};
}



#endif // !defined(AFX_AUTOLOCK_H__DB81A533_6AB7_4316_ADCC_D4C2D31552D9__INCLUDED_)

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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

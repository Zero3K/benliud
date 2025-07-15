/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
// BTListenSpeedControl.h: interface for the CBTListenSpeedControl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTLISTENSPEEDCONTROL_H
#define _BTLISTENSPEEDCONTROL_H

#include "SpeedControlBase.h"
#include <Mutex.h>
#include <list>

class CBTListenSpeedControl : public CSpeedControlBase
{
	typedef std::list<CBTPeer*> TClientList;
public:

	void Update();
	CBTListenSpeedControl();
	virtual ~CBTListenSpeedControl();
	void RegisteClient(CBTPeer* client);
	void UnregisteClient(CBTPeer* client);

protected:
	void Download();
	void Upload();
	void CleanClient();
    TClientList m_ClientList;
	TClientList m_PendingList;
    SockLib::CMutex m_PendingListMutex;
};

#endif // !defined(AFX_BTLISTENSPEEDCONTROL_H__D4E6559B_07C2_414F_9789_BBBE2707103E__INCLUDED_)

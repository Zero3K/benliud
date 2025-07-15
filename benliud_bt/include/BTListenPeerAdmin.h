/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// BTListenPeerAdmin.h: interface for the CBTListenPeerAdmin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTLISTENPEERADMIN_H
#define _BTLISTENPEERADMIN_H

#include "PeerAdminBase.h"
#include "BTListener.h"
#include "TPeerInfo.h"
#include <TimerClient.h>
#include <map>
#include <list>
class CBTListenPeerAdmin : public SockLib::CTimerClient , public CPeerAdminBase
{
	//typedef std::map<std::string, TPeerInfo> TPeerInfoMap;
	typedef std::list<CBTPeer*> TPeerList;

public:
	bool NewAccept(int handle, unsigned int iip, unsigned short iport);
	void Stop();
	void Start();
	virtual void OnTimer(unsigned int id);
	CBTListenPeerAdmin(CBTListener* parent);
	virtual ~CBTListenPeerAdmin();

	SockLib::CDealer* GetDealer();

	CSpeedControlBase* GetSpeedControl();
	CBTSession*	GetSession();
	//当接收的连接获得infohash时调用这个，完了以后即可以判断是关闭还是转移这个连接
	bool GotHash(std::string hash, CBTPeer* client);
	bool GotEncryptHash(std::string hashxor, MSE::BigInt dhsecret, CBTPeer* client);
protected:
	void CheckClosedConnection();
	unsigned int m_nCheckTimer;
	CBTListener *m_pParent;
	TPeerList m_ConnectedPeerList;
};

#endif // !defined(AFX_BTLISTENPEERADMIN_H__E000125F_C9CE_4B7A_94A1_024332324591__INCLUDED_)

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
// BTListenSocket.h: interface for the CBTListenSocket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTLISTENSOCKET_H
#define _BTLISTENSOCKET_H

#include <TCPServerSock.h>

#include "BTListener.h"

class CBTListenSocket : public SockLib::CTCPServerSock 
{
public:
	void Stop();
	bool Start(unsigned int port);
	virtual void OnClose();
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnTimer(unsigned int);
	CBTListenSocket(CBTListener* parent);
	virtual ~CBTListenSocket();
protected:
	CBTListener *m_pParent;
};

#endif // !defined(AFX_BTLISTENSOCKET_H__944DF0DD_B7B0_4255_B44D_E662FA78A9D8__INCLUDED_)

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

// BTListenSocket.cpp: implementation of the CBTListenSocket class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif
#include "../include/BTListenSocket.h"
#include "../include/BTListener.h"
#include <Dealer.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern void syslog( std::string info );

CBTListenSocket::CBTListenSocket(CBTListener* parent)
{
	m_pParent=parent;
}

CBTListenSocket::~CBTListenSocket()
{

}

void CBTListenSocket::OnRead()
{
	CTCPServerSock::OnRead();
	if(!CanRead()) return;

	SockLib::TInetAddr4 dest;

	int fd=Accept(dest);

	if(fd<=0)
	{
#ifdef WIN32
		Sleep(200);
#else
                Tools::Sleep(200);
#endif
		return;
	}

	m_pParent->NewAccept(fd,dest.iip,dest.iport);
}

void CBTListenSocket::OnWrite()
{
	CTCPServerSock::OnWrite();
}

void CBTListenSocket::OnTimer(unsigned int id)
{
	CTCPServerSock::OnTimer(id);
}

void CBTListenSocket::OnClose()
{
	CTCPServerSock::OnClose();
}

bool CBTListenSocket::Start(unsigned int port)
{
	//start the listen socket

	if(!CreateSock()) {
		printf("listen sock failed\n");
		return false;
	}
	
	if(!Bind(0, htons(port), true))
	{//may fail
		printf("listen sock bind failed\n");
		Close();
		return false;
	}

	if(!Listen()) {
		printf("listen sock listen failed\n");
		Close();
		return false;
	}

	maskRead(true);
	maskWrite(false);

	return true;
}

void CBTListenSocket::Stop()
{
	CTCPServerSock::Close();
}

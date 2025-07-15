/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/Sock.h"
#include "../include/Dealer.h"

namespace SockLib
{	
	void CSock::SetDealer( CDealer* dealer ) 
	{ 
		if(m_pDealer!=NULL) {
			m_pDealer->RemoveSockClient(this);
			m_pDealer->RemoveTimerClient(this);
		}
		m_pDealer=dealer;
		if(m_pDealer!=NULL&&m_hSocket!=-1) m_pDealer->AddSockClient(this);
	}

	unsigned int CSock::AddTimer(unsigned int interval, bool oneshot)
	{
		assert(m_pDealer!=NULL) ; //fail
		return m_pDealer->AddTimer(this,interval,oneshot);
	}


	void CSock::RemoveTimer(unsigned int id)
	{
		if(m_pDealer==NULL) return; //assert(0);
		m_pDealer->RemoveTimer(id);
	}


	void CSock::Attach(int fd)
	{
		assert(m_hSocket==-1);
		m_hSocket = fd;
		SetBlock(false);
		m_bMaskRead=false;
		m_bMaskWrite=false;
		if(m_pDealer!=NULL)
		{
			m_pDealer->AddSockClient(this);
		}

	}


	void CSock::Close()
	{
		if ( m_hSocket != -1 )
		{


#ifdef WIN32
			closesocket( m_hSocket );
#else

			::close( m_hSocket );
#endif

			m_hSocket = -1;

			m_bMaskRead=false;
			m_bMaskWrite=false;
		}		

		if(m_pDealer!=NULL)
		{
			m_pDealer->RemoveSockClient( this );
			m_pDealer->RemoveTimerClient(this);
		}
	}

	//linger=true, set to hard close
	void CSock::SetLinger(bool linger)
	{
#ifdef WIN32
		setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)(&linger), sizeof(bool));
#endif
	}
}
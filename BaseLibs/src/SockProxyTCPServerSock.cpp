/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/SockProxyTCPServerSock.h"

namespace SockLib
{	
	bool CSockProxyTCPServerSock::Bind(unsigned int iip ,unsigned short iport, bool reuse)
	{
		if(!m_bUseProxy) 
		{
			return CTCPServerSock::Bind(iip,iport,reuse);
		}
		else 
		{
			m_ListenPort=iport;
			return ConnectProxy(TInetAddr4(m_SocketProxyIP,m_SocketProxyPort),20*1000);
		}
	}

	void CSockProxyTCPServerSock::SetSocketProxy(SOCKSTYPE type,unsigned int iip, unsigned short iport)
	{
		m_bUseProxy=true;
		m_SocksType=type;
		m_SocketProxyIP=iip;
		m_SocketProxyPort=iport;
	}

	bool CSockProxyTCPServerSock::ConnectProxy(TInetAddr4 dest, unsigned int timeout)
	{
		assert(m_pDealer!=NULL);
		assert(m_hSocket!=-1);

		struct sockaddr_in addr;
		memset( &addr, 0, sizeof( addr ) );

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = dest.iip;
		addr.sin_port = dest.iport;

#ifdef WIN32

		connect( m_hSocket, ( const sockaddr* ) & addr, sizeof( addr ) );
#else

		::connect( m_hSocket, ( const sockaddr* ) & addr, ( socklen_t ) ( sizeof( addr ) ) );
#endif

		maskWrite(true);
		maskRead(false);

		m_nConnectProxyTimeOut=AddTimer(timeout,true);

		return true;
	}

	void CSockProxyTCPServerSock::OnTimer(unsigned int id)
	{
		CTCPServerSock::OnTimer(id);
		if(id==m_nConnectProxyTimeOut)
		{
			m_SockLink=_SOCKFAIL;		
			OnBind(false);
		}
	}

	void CSockProxyTCPServerSock::OnBind(bool ok)
	{

	}

	void CSockProxyTCPServerSock::OnRead()
	{

	}

	void CSockProxyTCPServerSock::OnWrite()
	{

	}

	void CSockProxyTCPServerSock::OnClose()
	{
	}

}
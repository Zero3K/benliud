/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

//GSSAPI document :
//http://tools.ietf.org/html/rfc1961
//http://www.faqs.org/rfcs/rfc4752.html

#include "../include/SockProxyTCPClientSock.h"
#include <stdio.h>

namespace SockLib
{	
	//for app, OnConnectOk don't mean can write, it should call canwrite to test.
	//when proxy have finished the shake, it will call onconnectok again.
	void CSockProxyTCPClientSock::OnConnectOk()
	{//do our proxy connect...
		if(!m_bUseProxy) 
		{
			m_bProxyCanWrite=true;
			maskWrite(false);
		}
		else
		{
			maskWrite(false);

			if(m_SockLink==_SOCKCONN)
			{//do the shake to proxy.
				//send login packet to proxy
				//set a timer to guard timeout event
				printf("do proxy connect\n");

				switch(m_SocksType)
				{
				case _SOCKSAUTO:
				case _SOCKS5:
					DoV5Shake();
					break;
				case _SOCKS4A:
					DoV4AConnect(m_PeerAddr.c_str(),ntohs(m_PeerPort));
					break;
				case _SOCKS4:
					if(m_PeerIP!=0)
					{
						DoV4Connect(m_PeerIP,m_PeerPort);
					}
					else
					{
						DoV4Connect(m_PeerAddr.c_str(),ntohs(m_PeerPort));
					}
					break;
				}

			}
			else if(m_SockLink==_SOCKSUCC)
			{
				m_bProxyCanWrite=true;
			}
		}
	}

	void CSockProxyTCPClientSock::OnConnectFail()
	{
		m_bProxyCanWrite=false;
		m_bProxyCanRead=false;
		Close();
	}

	void CSockProxyTCPClientSock::OnRead()
	{
		CTCPClientSock::OnRead();
		//only set m_bProxyCanRead=true if proxy connect ok;
		//else set m_bProxyCanRead=false

		if(m_bUseProxy)
		{
			switch(m_SockLink)
			{
			case _SOCKINIT:
				assert(false);
				break;
			case _SOCKCONN:
				assert(false);
				break;
			case _SOCKV4CONN:
			case _SOCKV4ACONN:
				if(CheckV4ConnResponse())
				{
					m_SockLink=_SOCKSUCC;
					OnConnectOk();
				}
				else
				{
					OnConnectFail();
				}
				break;
			case _SOCKV5SHAKE:
				if(CheckV5ShakeResponse())
				{
					if(m_SockLink==_SOCKV5CONN) //²»ÐèÒªµÇÂœ£¬Ö±œÓÁ¬œÓ
					{
						if(m_PeerIP!=0)
						{
							DoV5Connect(m_PeerIP,m_PeerPort);
						}
						else
						{
							DoV5Connect(m_PeerAddr.c_str(),ntohs(m_PeerPort));
						}
					}

				}
				else
				{//²»Ö§³ÖµÄ°æ±Ÿ£¬¿ŒÂÇœµµÍ°æ±Ÿ
					if(m_SocksType==_SOCKSAUTO)
					{
						if(m_PeerIP!=0)
						{
							DoV4AConnect(m_PeerIP,m_PeerPort);
						}
						else
						{
							DoV4AConnect(m_PeerAddr.c_str(),ntohs(m_PeerPort));
						}
					}
					else
					{
						OnConnectFail();
					}
				}
				break;
			case _SOCKV5LOG:
				if(CheckV5LogResponse())
				{
					if(m_PeerIP!=0)
						DoV5Connect(m_PeerIP,m_PeerPort);
					else
						DoV5Connect(m_PeerAddr.c_str(),ntohs(m_PeerPort));
				}
				else
				{
					OnConnectFail();
				}
				break;
			case _SOCKV5CONN:
				if(CheckV5ConnResponse())
				{
					m_SockLink=_SOCKSUCC;
					OnConnectOk();
				}
				else
				{
					OnConnectFail();
				}
				break;
			case _SOCKFAIL:
				m_bProxyCanRead=false;
				break;
			case _SOCKSUCC:
				m_bProxyCanRead=true;
				break;
			}
		}
		else
		{
			m_bProxyCanRead=true;
		}
	}

	void CSockProxyTCPClientSock::OnWrite()
	{
		CTCPClientSock::OnWrite();

		if(!m_bUseProxy) m_bProxyCanWrite=true;
		else if(_SOCKSUCC==m_SockLink) m_bProxyCanWrite=true;

		maskWrite(false);
	}

	bool CSockProxyTCPClientSock::Connect(TInetAddr4 dest, unsigned int timeout)
	{

		if(!m_bUseProxy)
		{
			return CTCPClientSock::Connect(dest.iip,dest.iport,timeout);
		}
		else
		{
			/*		
			#ifdef WIN32
			m_PeerAddr = inet_ntoa( *((in_addr*)(&iip)) );
			#else
			const char *ret;
			char ipbuf[ INET_ADDRSTRLEN ];
			m_PeerAddr = inet_ntop( AF_INET, (const void*)(&dest.iip), ipbuf, INET_ADDRSTRLEN );
			#endif
			*/
			m_PeerIP=dest.iip;
			m_PeerPort=dest.iport;
			m_SockLink=_SOCKCONN;
			m_nTimeOut=timeout;

			return CTCPClientSock::Connect(m_SocketProxyIP,m_SocketProxyPort,timeout);
		}

	}



	bool CSockProxyTCPClientSock::Connect(std::string dest, unsigned short port, unsigned int timeout)
	{
		if(!m_bUseProxy)
		{//²»ÓÃŽúÀíÊ±±ÜÃâÓÃÕâžöº¯Êý

			struct hostent *hp;

			hp = gethostbyname( dest.c_str() );

			if(hp==NULL) return false;
			if(hp->h_addrtype!=AF_INET) return false;


			m_PeerIP= * (( unsigned int* ) hp->h_addr ); //ÍøÂçÐò

			m_PeerPort=htons(port);
			m_nTimeOut=timeout;

			CTCPClientSock::Connect(m_PeerIP,m_PeerPort,m_nTimeOut);

			return true;
		}
		else
		{

			m_PeerAddr=dest;
			m_PeerPort=htons(port);
			m_SockLink=_SOCKCONN;
			m_nTimeOut=timeout;

			CTCPClientSock::Connect(m_SocketProxyIP,m_SocketProxyPort,timeout);

			return true;
		}
	}





	void CSockProxyTCPClientSock::SetSocketProxy(SOCKSTYPE type,unsigned int iip, unsigned short iport)
	{
		m_bUseProxy=true;
		m_SocksType=type;
		m_SocketProxyIP=iip;
		m_SocketProxyPort=iport;
		m_SocketUser="";
		m_SocketPass="";
	}

	void CSockProxyTCPClientSock::DoV5Shake()
	{
		char shake[ 4 ] = {0x05, 0x02, 0x00, 0x02};

		int nret=Send(shake, 4);

		if(nret <=0) {
			OnClose(); return;
		}

		m_SockLink=_SOCKV5SHAKE;
		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);
		maskRead(true);
	}

	//connect with ip/port.
	void CSockProxyTCPClientSock::DoV5Connect(unsigned int iip, unsigned short iport )
	{
		unsigned char buf[ 512 ];

		buf[ 0 ] = 0x05;
		buf[ 1 ] = 0x01; //connect
		buf[ 2 ] = 0x00; //reserve
		buf[ 3 ] = 0x01; //0x03 for domain, 0x01 for ipv4, 0x04 for ipv6
		buf[ 4 ] = sizeof(unsigned int);

		memcpy( buf + 5, &iip, sizeof(unsigned int) );
		memcpy( buf + 5 + sizeof(unsigned int), &iport, sizeof(unsigned short) );

		int nret=Send((char*)buf, 5 + sizeof(unsigned int) + sizeof(unsigned short));

		if(nret <=0) {
			OnClose(); return;
		}

		m_SockLink=_SOCKV5CONN;

		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);
		maskRead(true);
	}

	//connct with domain name
	void CSockProxyTCPClientSock::DoV5Connect( const char* server , unsigned short port )
	{

		char buf[ 512 ];
		buf[ 0 ] = 0x05;
		buf[ 1 ] = 0x01; //connect
		buf[ 2 ] = 0x00; //reserve
		buf[ 3 ] = 0x03; //0x03 for domain, 0x01 for ipv4, 0x04 for ipv6
		buf[ 4 ] = strlen( server );
		memcpy( buf + 5, server, strlen( server ) );
		int pos = 5 + buf[ 4 ];
		unsigned short iport = htons( port );
		memcpy( buf + pos, &iport, 2 );
		pos += 2;

		int nret=Send(buf, pos);
		if(nret <=0) {
			OnClose(); return;
		}

		m_SockLink=_SOCKV5CONN;

		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);
		maskRead(true);

	}

	void CSockProxyTCPClientSock::DoV4Connect( unsigned int peerip , unsigned short iport )
	{

		//		+----+----+----+----+----+----+----+----+----+----+....+----+
		//		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		//		+----+----+----+----+----+----+----+----+----+----+....+----+
		//bytes:	   1    1      2              4           variable       1
		//	
		//	
		//		+----+----+----+----+----+----+----+----+
		//		| VN | CD | DSTPORT |      DSTIP        |
		//		+----+----+----+----+----+----+----+----+
		//bytes:	   1    1      2              4
		// 
		//VN is the version of the reply code and should be 0. CD is the result
		//code with one of the following values:
		// 
		//	90: request granted
		//	91: request rejected or failed
		//	92: request rejected becasue SOCKS server cannot connect to
		//	    identd on the client
		//	93: request rejected because the client program and identd
		//	    report different user-ids.	


		char buf[ 256 ];

		buf[ 0 ] = 0x04;
		buf[ 1 ] = 0x01;
		memcpy( buf + 2, &iport, 2 );
		memcpy( buf + 4, &peerip, 4 );
		memcpy( buf + 8, m_SocketUser.data(), m_SocketUser.size() );
		buf[ 8 + m_SocketUser.size() ] = 0;

		int nret=Send (buf, 8 + m_SocketUser.size() + 1);
		assert(nret==8 + m_SocketUser.size() + 1);

		m_SockLink=_SOCKV4CONN;

		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);
	}

	bool CSockProxyTCPClientSock::DoV4Connect( const char* peer , unsigned short port )
	{ //resolv ip locally

		//		+----+----+----+----+----+----+----+----+----+----+....+----+
		//		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		//		+----+----+----+----+----+----+----+----+----+----+....+----+
		//bytes:	   1    1      2              4           variable       1
		//	
		//	
		//		+----+----+----+----+----+----+----+----+
		//		| VN | CD | DSTPORT |      DSTIP        |
		//		+----+----+----+----+----+----+----+----+
		//bytes:	   1    1      2              4
		// 
		//VN is the version of the reply code and should be 0. CD is the result
		//code with one of the following values:
		// 
		//	90: request granted
		//	91: request rejected or failed
		//	92: request rejected becasue SOCKS server cannot connect to
		//	    identd on the client
		//	93: request rejected because the client program and identd
		//	    report different user-ids.	



		char buf[ 256 ];

		struct hostent *hp;

		hp = gethostbyname( peer );

		if ( !hp )
		{
			return false;
		}

		if ( hp->h_addrtype != AF_INET )
		{ //neither AF_INET nor AF_INET6

			return false;
		}

		unsigned short iport = htons( port );

		buf[ 0 ] = 0x04;
		buf[ 1 ] = 0x01;
		memcpy( buf + 2, &iport, 2 );
		memcpy( buf + 4, ( void* ) ( hp->h_addr ), 4 );
		memcpy( buf + 8, m_SocketUser.data(), m_SocketUser.size() );
		buf[ 8 + m_SocketUser.size() ] = 0;

		int nret=Send (buf, 8 + m_SocketUser.size() + 1);
		//assert(nret==8 + m_SocketUser.size() + 1);

		if(nret <=0) {
			return false;
		}

		m_SockLink=_SOCKV4CONN;

		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);
		maskRead(true);

		return true;
	}

	void CSockProxyTCPClientSock::DoV4AConnect( const char* peer , unsigned short port )
	{

		char buf[ 256 ];

		buf[ 0 ] = 0x04; //version 4
		buf[ 1 ] = 0x01; //connect command, 1 for connect, 2 for bind

		//port
		unsigned short iport = htons( port );
		memcpy( buf + 2, &iport, 2 );

		//ip addr, first three bytes should = 0
		//the last byte should !=0
		buf[ 4 ] = buf[ 5 ] = buf[ 6 ] = 0;
		buf[ 7 ] = 0x01;

		//userid terminated with 0
		memcpy( buf + 8, m_SocketUser.data(), m_SocketUser.size() );
		buf[ 8 + m_SocketUser.size() ] = 0;

		//domain terminated with 0
		memcpy( buf + 8 + m_SocketUser.size() + 1, peer, strlen( peer ) );
		buf[ 8 + m_SocketUser.size() + 1 + strlen( peer ) ] = 0;

		int nret=Send( buf, 8 + m_SocketUser.size() + 1 + strlen( peer ) + 1);

		if(nret <=0) {
			OnClose(); return;
		}

		m_SockLink=_SOCKV4ACONN;

		//make a timer to guard timeout
		m_SockLinkTimer=AddTimer(m_nTimeOut, true);

		maskRead(true);
	}

	void CSockProxyTCPClientSock::DoV4AConnect( unsigned int iip , unsigned short iport )
	{

		DoV4Connect(iip,iport);
	}

	void CSockProxyTCPClientSock::OnTimer(unsigned int id)
	{

		CTCPClientSock::OnTimer(id);


		if(m_bUseProxy && m_SockLinkTimer==id)
		{

			switch(m_SockLink)
			{
			case _SOCKINIT: //should not happen
				break;
			case _SOCKCONN:
				break;
			case _SOCKV4CONN:
			case _SOCKV4ACONN:
				{
					printf("do v4 conn timeout, fail\n");
					OnConnectFail();	//Ã»ÓÐÁ¬œÓÉÏŽúÀí
				}
				break;
			case _SOCKV5SHAKE:
				{
					printf("sock5 shake timeout, fail\n");
					OnConnectFail();
				}
				break;
			case _SOCKV5LOG:
				{
					printf("sock5 login timeout, fail\n");
					OnConnectFail();
				}
				break;
			case _SOCKV5CONN:
				{
					printf("sock5 conn timeout, fail\n");
					OnConnectFail();
				}
				break;
			case _SOCKFAIL:
				break;
			case _SOCKSUCC:
				break;
			}

			m_SockLinkTimer=0;
		}
	}



	bool CSockProxyTCPClientSock::CheckV4ConnResponse()
	{

		RemoveTimer(m_SockLinkTimer);
		m_SockLinkTimer=0;

		char buf[32];

		int nret = Recv( buf, 32 );

		if(nret <=0) {
			return false;
		}

		if ( nret != 8 )
		{
			printf("CheckV4ConnResponse fail-1,nret=%d\n",nret);
			return false;
		}

		if ( buf[ 0 ] != 0 )
		{
			return false;
		}

		if ( buf[ 1 ] == 90 )
		{
			return true;
		}
		else if ( buf[ 1 ] == 91 )
		{
			return false;
		}
		else if ( buf[ 1 ] == 92 )
		{
			return false;
		}
		else if ( buf[ 1 ] == 93 )
		{
			return false;
		}

		return false;
	}

	bool CSockProxyTCPClientSock::CheckV5ShakeResponse()
	{
		RemoveTimer(m_SockLinkTimer);
		m_SockLinkTimer=0;

		unsigned char buf[ 32 ];

		int nret = Recv( (char*)buf, 32 );

		if(nret <=0) {
			OnClose(); return false;
		}

		if ( nret != 2 )
		{
			printf("CheckV5ShakeResponse fail-1, nret=%d\n",nret);
			return false;
		}

		if ( buf[ 0 ] != 0x05 )
		{
			printf("CheckV5ShakeResponse fail-2\n");
			return false;
		}

		if ( buf[ 1 ] == 0xFF )
		{
			printf("CheckV5ShakeResponse fail-3\n");
			return false;
		}

		if ( buf[ 1 ] != 0 && buf[ 1 ] != 1 && buf[ 1 ] != 2 )
		{
			printf("CheckV5ShakeResponse fail-4\n");
			return false;
		}

		if ( buf[ 1 ] == 0 )
		{ //ok, no password need ,finished login
			printf("no pass need\n");
			m_SockLink=_SOCKV5CONN; //±êŒÇ×ŽÌ¬Ö±œÓµœÁ¬œÓÌ¬£¬±íÊŸ²»ÐèÒªµÇÂœÁË

			return true;
		}
		else if ( buf[ 1 ] == 1 )
		{ //GSSAPI
			printf("gssapi ,fail.\n");
			return false; //Ã»ÓÐÕâžöŒÓÃÜÄÜÁŠ
		}
		else if ( buf[ 2 ] == 2)
		{//ÓÃ»§Ãû/ÃÜÂë·œÊœ£¬·¢ËÍ°ü

			printf("u/p login\n");

			int tl = 0;
			buf[ 0 ] = 0x01;
			tl++;
			buf[ tl ] = m_SocketUser.size();
			tl++;
			memcpy( buf + tl, m_SocketUser.data(), m_SocketUser.size() );
			tl += m_SocketUser.size();
			buf[ tl ] = m_SocketPass.size();
			tl++;
			memcpy( buf + tl, m_SocketPass.data(), m_SocketPass.size() );
			tl += m_SocketPass.size();

			Send((char*)buf, tl);

			m_SockLink=_SOCKV5LOG;

			m_SockLinkTimer=AddTimer(m_nTimeOut, true);

			maskRead(true);

			return true;
		}
		else
		{
			printf("unknown sock5 auth method\n");
			return false;
		}
	}


	bool CSockProxyTCPClientSock::CheckV5LogResponse()
	{
		RemoveTimer(m_SockLinkTimer);
		m_SockLinkTimer=0;

		char buf[ 64 ];

		int nret = Recv(buf, 64 );

		if(nret <=0) {
			OnClose(); return false;
		}

		if(nret!=2)
		{
			return false;
		}

		if ( buf[ 0 ] != 0x01 )
		{

			return false;
		}

		if ( buf[ 1 ] != 0x00 )
		{

			return false;
		}

		return true;
	}

	bool CSockProxyTCPClientSock::CheckV5ConnResponse()
	{
		RemoveTimer(m_SockLinkTimer);
		m_SockLinkTimer=0;

		char buf[32];

		int nret = Recv( buf, 32 );

		if ( nret <= 0 )
		{
			return false;
		}

		if ( nret < 10 )
		{
			return false;
		}

		if ( buf[ 0 ] != 0x05 || buf[ 2 ] != 0x00 )
		{
			return false;
		}


		//# X'00' success
		//# X'01' fail
		//# X'02' not allow
		//# X'03' net unreach
		//# X'04' host unreach
		//# X'05' connect refuse
		//# X'06' TTL timeout
		//# X'07' not support command
		//# X'08' not support address
		//# X'09' â?X'FF' undef

		if ( buf[ 1 ] == 0 )
		{
			return true;
		}
		else if ( buf[ 1 ] == 0x01 )
		{
			return false;
		}
		else if ( buf[ 1 ] == 0x02 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x03 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x04 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x05 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x06 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x07 )
		{

			return false;
		}
		else if ( buf[ 1 ] == 0x08 )
		{

			return false;
		}
		else
		{

			return false;
		}

		return false;
	}

	void CSockProxyTCPClientSock::OnClose()
	{
		CTCPClientSock::OnClose();
		m_bProxyCanRead=false;
		m_bProxyCanWrite=false;
	}

}
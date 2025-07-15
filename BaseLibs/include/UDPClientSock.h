/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _UDPCLIENTSOCK_HPP_
#define _UDPCLIENTSOCK_HPP_

#include "ClientSock.h"
#include "Dealer.h"
namespace SockLib
{
	class CUDPClientSock: public CClientSock 
	{
	public:
		bool CreateSock()
		{
			assert(m_hSocket==-1);
			m_hSocket = socket( AF_INET, SOCK_DGRAM,0 );

			if(m_hSocket <=0) return false;

			SetBlock(false);
			maskRead(false);
			maskWrite(false);
			if(m_pDealer!=NULL) m_pDealer->AddSockClient(this);
			return true;	
		}

		virtual int Send( const char* buf, int len,unsigned int iip, unsigned short iport) 
		{
			struct sockaddr_in dest;
			dest.sin_family = PF_INET;

#ifdef WIN32
			dest.sin_addr.S_un.S_addr = iip;
#else
			dest.sin_addr.s_addr = iip;
#endif

			dest.sin_port = iport;


			return sendto(m_hSocket, buf, len, 0, ( const struct sockaddr* ) & dest, sizeof( dest ) );

		}

		virtual int Recv(char* buf, int len,unsigned int& iip, unsigned short& iport)
		{
			struct sockaddr_in addr;
			memset( &addr, 0, sizeof( addr ) );

#ifdef WIN32
			int addrlen = sizeof( addr );
#else
			socklen_t addrlen = ( socklen_t ) sizeof( addr );
#endif

			int nret = recvfrom( m_hSocket, buf, len, 0, ( struct sockaddr* ) & addr, &addrlen );

#ifdef WIN32
			iip=addr.sin_addr.S_un.S_addr;
#else
			iip=addr.sin_addr.s_addr;
#endif
			iport=addr.sin_port;

			return nret;

		}

	protected:

	private:

	};
}
#endif // _UDPCLIENTSOCK_HPP_

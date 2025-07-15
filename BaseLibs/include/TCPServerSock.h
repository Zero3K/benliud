/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _TCPSERVERSOCK_H_
#define _TCPSERVERSOCK_H_

#include "ServerSock.h"
#include "Dealer.h"

namespace SockLib
{
	class CTCPServerSock: public CServerSock 
	{
	public:
		CTCPServerSock()
			:m_bSockCanRead(false),
			m_bSockCanWrite(false){}

		int Accept(TInetAddr4& peeraddr)
		{

			struct sockaddr_in addr;
#ifdef WIN32

			int len = sizeof( addr );
#else

			socklen_t len = ( socklen_t ) ( sizeof( addr ) );
#endif

			memset( &addr, 0, sizeof( addr ) );

			int fd = ::accept( m_hSocket, ( struct sockaddr* ) & addr, &len );

			if ( fd >0 )
			{
				peeraddr.iip = *((unsigned int*)(&addr.sin_addr)) ;
				peeraddr.iport =  addr.sin_port ;
			}

			return fd;
		}

		virtual bool CreateSock()
		{
			assert(m_hSocket==-1);
			m_hSocket = socket( AF_INET, SOCK_STREAM,0 );

			if(m_hSocket <=0) return false;

			SetBlock(false);
			maskRead(false);
			maskWrite(false);

			if(m_pDealer!=NULL) m_pDealer->AddSockClient(this);
			return true;		
		}

		virtual bool Listen()
		{
			return listen(m_hSocket,5)==0;
		}

		virtual int Send(char* buf, int len)
		{
			assert(m_hSocket!=-1);
			return send(m_hSocket,buf,len,0);
		}

		virtual int Recv(char* buf, int len)
		{
			assert(m_hSocket!=-1);
			return recv(m_hSocket,buf,len,0);
		}

		virtual void OnClose()
		{
		}

		virtual void OnRead() {m_bSockCanRead=true;}
		virtual void OnWrite(){m_bSockCanWrite=true;}

		virtual bool CanRead(){return m_bSockCanRead;}
		virtual bool CanWrite(){return m_bSockCanWrite;}
	protected:
		bool m_bSockCanRead;
		bool m_bSockCanWrite;
	private:

	};
}
#endif // _CTCPSERVERSOCK_H_

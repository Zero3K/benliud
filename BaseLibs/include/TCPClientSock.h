/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _TCPCLIENTSOCK_H_
#define _TCPCLIENTSOCK_H_

#include "ClientSock.h"
#include "Dealer.h"

namespace SockLib
{
	class CTCPClientSock: public CClientSock 
	{
	public:
		CTCPClientSock():m_nConnectTimeOut(0),m_bSockCanWrite(false),m_bSockCanRead(false){};
		virtual void Attach(int fd) {
			CClientSock::Attach(fd);
			m_bSockCanRead=true;
			m_bSockCanWrite=true;
		}
		virtual void OnConnectFail()=0;
		virtual void OnConnectOk()=0;
		virtual bool CanWrite() {return m_bSockCanWrite;}
		virtual bool CanRead() {return m_bSockCanRead;}

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

		virtual bool Connect(unsigned int iip, unsigned short iport, unsigned int timeout)
		{
			TInetAddr4 dest;
			dest.iip=iip;
			dest.iport=iport;
			return CTCPClientSock::Connect(dest,timeout);
		}

		virtual bool Connect(TInetAddr4 dest, unsigned int timeout)
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

			m_nConnectTimeOut=AddTimer(timeout,true);

			return true;
		}

		virtual void OnTimer(unsigned int id)
		{
			//CClientSock::OnTimer(id);
			if(id==m_nConnectTimeOut)
			{
				Close();
				m_nConnectTimeOut=0;
				OnConnectFail();
			}
		}

		virtual void OnWrite()
		{

			CClientSock::OnWrite();
			if(m_nConnectTimeOut!=0)
			{//connecting
				RemoveTimer(m_nConnectTimeOut);
				m_nConnectTimeOut=0;
				m_bSockCanWrite=true;
				OnConnectOk();
			}
			else
			{
				m_bSockCanWrite=true;
			}
		}

		virtual void OnRead()
		{
			CClientSock::OnRead(); //in fact it's empty
			m_bSockCanRead=true;
			/*
			//guard close event
			char buf[2];
			if(0==recv(m_hSocket,buf,2,MSG_PEEK))
			{
			OnClose();
			}
			else
			{
			m_bSockCanRead=true;
			}
			*/
		}

		virtual void OnClose()
		{
			m_bSockCanRead=false;
			m_bSockCanWrite=false;
			CSock::Close();
		}

		virtual int Send(char* buf, int len)
		{
			assert(m_hSocket!=-1);
			assert(m_bSockCanWrite);

#ifdef WIN32
			return send(m_hSocket,buf,len,0);
#else
			return send(m_hSocket,buf,len,MSG_NOSIGNAL);
#endif
		}

		virtual int Recv(char* buf, int len)
		{
			assert(m_hSocket!=-1);
			assert(m_bSockCanRead);
			return recv(m_hSocket,buf,len,0);
		}

	protected:
		unsigned int m_nConnectTimeOut;
		bool m_bSockCanWrite;
		bool m_bSockCanRead;
	private:

	};
}
#endif // _TCPCLIENTSOCK_H_

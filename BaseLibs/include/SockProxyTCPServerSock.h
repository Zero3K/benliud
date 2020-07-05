/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _SOCKPROXYTCPSERVERSOCK_H_
#define _SOCKPROXYTCPSERVERSOCK_H_

#include "TCPServerSock.h"
namespace SockLib
{
	class CSockProxyTCPServerSock: public CTCPServerSock 
	{
		enum SOCKLINK{_SOCKINIT,_SOCKCONN,_SOCKV4CONN,_SOCKV4ACONN,_SOCKV5SHAKE,_SOCKV5LOG,_SOCKV5CONN,_SOCKFAIL,_SOCKSUCC};
		enum SOCKSTYPE{_SOCKSAUTO,_SOCKS4,_SOCKS4A,_SOCKS5};
	public:
		CSockProxyTCPServerSock()
			:m_bUseProxy(false),
			m_bProxyCanRead(false),
			m_bProxyCanWrite(false),
			m_SockLink(_SOCKINIT),
			m_ListenPort(0),
			m_nConnectProxyTimeOut(0){}

		virtual bool CanWrite() {return m_bProxyCanWrite;}
		virtual bool CanRead() {return m_bProxyCanRead;}

		void OnConnectProxyOk();

		virtual void OnBind(bool ok);
		virtual void OnTimer(unsigned int id);
		virtual void OnRead();
		virtual void OnWrite();
		virtual void OnClose();
		//virtual bool Connect(TInetAddr4 dest, unsigned int timeout);
		//virtual bool Connect(std::string dest, unsigned short port, unsigned int timeout);

		void DoV5Shake();
		void DoV5Connect( unsigned int iip, unsigned short iport );
		void DoV5Connect( const char* peer , unsigned short port );
		bool DoV4Connect( const char* peer , unsigned short port );
		void DoV4Connect( unsigned int iip , unsigned short iport );
		void DoV4AConnect( const char* peer , unsigned short port );
		void DoV4AConnect( unsigned int iip , unsigned short iport );
		bool CheckV5ConnResponse();
		bool CheckV5LogResponse();
		bool CheckV5ShakeResponse();
		bool CheckV4ConnResponse();

		void SetSocketProxy(SOCKSTYPE type,unsigned int iip, unsigned short iport);
		virtual bool Bind(unsigned int iip ,unsigned short iport, bool reuse);
		bool ConnectProxy(TInetAddr4 dest, unsigned int timeout);
	protected:
		bool m_bUseProxy;
		SOCKSTYPE m_SocksType;
		unsigned int m_SocketProxyIP;
		unsigned int m_SocketProxyPort;
		bool m_bProxyCanWrite;
		bool m_bProxyCanRead;

		SOCKLINK	m_SockLink;	//socket ŽúÀíÁ¬œÓÔËÐÐ×ŽÌ¬
		unsigned int m_SockLinkTimer;
		unsigned short m_ListenPort;
		std::string	m_SocketUser;
		std::string	m_SocketPass;
	private:
		unsigned int m_nConnectProxyTimeOut;
	};
}
#endif // _SOCKPROXYTCPSERVERSOCK_H_

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#ifndef _SOCKPROXYTCPCLIENTSOCK_H_
#define _SOCKPROXYTCPCLIENTSOCK_H_

#include "TCPClientSock.h"

namespace SockLib
{
	enum SOCKSTYPE{_SOCKSAUTO,_SOCKS4,_SOCKS4A,_SOCKS5};

	class CSockProxyTCPClientSock: public CTCPClientSock 
	{
		enum SOCKLINK{_SOCKINIT,_SOCKCONN,_SOCKV4CONN,_SOCKV4ACONN,_SOCKV5SHAKE,_SOCKV5LOG,_SOCKV5CONN,_SOCKFAIL,_SOCKSUCC};

	public:
		CSockProxyTCPClientSock()
			:m_bUseProxy(false),
			m_bProxyCanRead(false),
			m_bProxyCanWrite(false),
			m_SockLink(_SOCKINIT),
			m_PeerIP(0),
			m_PeerPort(0){}

		virtual void Attach(int fd) {
			CTCPClientSock::Attach(fd);
			m_bProxyCanRead=true;
			m_bProxyCanWrite=true;
			m_SockLink=_SOCKSUCC;
		}

		virtual bool CanWrite() {return m_bProxyCanWrite;}
		virtual bool CanRead() {return m_bProxyCanRead;}

		virtual void OnConnectOk();
		virtual void OnConnectFail();
		virtual void OnTimer(unsigned int id);
		virtual void OnRead();
		virtual void OnWrite();
		virtual void OnClose();
		virtual bool Connect(TInetAddr4 dest, unsigned int timeout);
		virtual bool Connect(std::string dest, unsigned short port, unsigned int timeout);

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
	protected:
		bool m_bUseProxy;
		SOCKSTYPE m_SocksType;
		unsigned int m_SocketProxyIP;
		unsigned int m_SocketProxyPort;
		bool m_bProxyCanWrite;
		bool m_bProxyCanRead;

		SOCKLINK	m_SockLink;	//socket ŽúÀíÁ¬œÓÔËÐÐ×ŽÌ¬
		unsigned int m_SockLinkTimer;
		unsigned int m_PeerIP;
		unsigned short m_PeerPort;
		std::string  m_PeerAddr; //ÓÃ×Ö·ûŽ®±íÊŸµØÖ·ÒòÎªŽúÀí¿ÉÄÜDNSœâÎö
		std::string	m_SocketUser;
		std::string	m_SocketPass;
	private:

	};
}
#endif // _SOCKPROXYTCPCLIENTSOCK_H_

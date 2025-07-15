/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/

#ifndef _SSLSOCKPROXYTCPCLIENTSOCK_H_
#define _SSLSOCKPROXYTCPCLIENTSOCK_H_

// SSL functionality disabled - using stub for compatibility  
// #include <openssl/ssl.h>
//#ifdef WIN32
//#pragma comment(lib,"ssleay32.lib")
//#pragma comment(lib,"libeay32.lib")
//#endif

#include "SockProxyTCPClientSock.h"

// Forward declarations for SSL stub
typedef void SSL_CTX;
typedef void SSL;
namespace SockLib
{
	class CSSLSockProxyTCPClientSock: public CSockProxyTCPClientSock 
	{
		enum SStatus{S_INIT,S_CONN,S_CONNOK,S_CONNFAIL,};

	public:
		CSSLSockProxyTCPClientSock()
			:m_bUseSSL(false),
			m_pSSL_CTX(NULL),
			m_pSSL_HANDLE(NULL),
			m_bSSLCanWrite(false),
			m_bSSLCanRead(false),
			m_SSL_Timer(0){}

		virtual ~CSSLSockProxyTCPClientSock();

		void SetSSL()
		{
			m_bUseSSL=true;
			m_SSL_STATUS=S_INIT;
		}

		virtual bool Connect(std::string dest, unsigned short port, unsigned int timeout);
		virtual bool Connect(TInetAddr4 addr, unsigned int timeout);
		virtual void OnRead();
		virtual void OnWrite();
		virtual void OnClose();
		virtual void OnConnectFail();
		virtual void OnConnectOk();
		virtual bool CanRead() {return m_bSSLCanRead;}
		virtual bool CanWrite() {return m_bSSLCanWrite;}
		virtual int Send(char* buf, int len);
		virtual int Recv(char* buf, int len);
		virtual void OnTimer(unsigned int id);
	protected:

		bool	m_bUseSSL;
		bool	m_bSSLCanWrite;
		bool	m_bSSLCanRead;

		SStatus	m_SSL_STATUS;
		SSL_CTX *m_pSSL_CTX;
		SSL 	*m_pSSL_HANDLE;

		unsigned int m_SSL_Timer;
	private:

	};
}
#endif // _SSLSOCKPROXYTCPCLIENTSOCK_H_

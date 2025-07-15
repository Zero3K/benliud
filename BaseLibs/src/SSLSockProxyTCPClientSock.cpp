/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/
#include "../include/SSLSockProxyTCPClientSock.h"

namespace SockLib
{	
	CSSLSockProxyTCPClientSock::~CSSLSockProxyTCPClientSock()
	{
		// Stub implementation - SSL functionality disabled
		m_pSSL_CTX = NULL;
		m_pSSL_HANDLE = NULL;
	}

	bool CSSLSockProxyTCPClientSock::Connect(std::string dest, unsigned short port, unsigned int timeout)
	{
		// Always use non-SSL connection for now
		return CSockProxyTCPClientSock::Connect(dest,port,timeout);
	}

	bool CSSLSockProxyTCPClientSock::Connect(TInetAddr4 addr, unsigned int timeout)
	{
		// Always use non-SSL connection for now
		return CSockProxyTCPClientSock::Connect(addr,timeout);
	}

	void CSSLSockProxyTCPClientSock::OnRead()
	{
		CSockProxyTCPClientSock::OnRead();
		if(CSockProxyTCPClientSock::CanRead())
		{
			m_bSSLCanRead=true;
		}
	}
	
	void CSSLSockProxyTCPClientSock::OnWrite()
	{
		CSockProxyTCPClientSock::OnWrite();
		if(CSockProxyTCPClientSock::CanWrite())
		{
			m_bSSLCanWrite=true;
		}
	}

	void CSSLSockProxyTCPClientSock::OnClose()
	{
		m_pSSL_CTX = NULL;
		m_pSSL_HANDLE = NULL;
		CSockProxyTCPClientSock::OnClose();
	}

	void CSSLSockProxyTCPClientSock::OnConnectFail()
	{
		m_pSSL_CTX = NULL;
		m_pSSL_HANDLE = NULL;
		CSockProxyTCPClientSock::OnConnectFail();
	}

	void CSSLSockProxyTCPClientSock::OnConnectOk()
	{
		CSockProxyTCPClientSock::OnConnectOk();
		if(CSockProxyTCPClientSock::CanWrite())
		{
			m_bSSLCanWrite=true;
		}
	}

	int CSSLSockProxyTCPClientSock::Send(char* buf, int len)
	{
		// Use non-SSL send
		return CSockProxyTCPClientSock::Send(buf,len);
	}

	int CSSLSockProxyTCPClientSock::Recv(char* buf, int len)
	{
		// Use non-SSL receive
		return CSockProxyTCPClientSock::Recv(buf,len);
	}

	void CSSLSockProxyTCPClientSock::OnTimer(unsigned int id)
	{
		CSockProxyTCPClientSock::OnTimer(id);
	}
}
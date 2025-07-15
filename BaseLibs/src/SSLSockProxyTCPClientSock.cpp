/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#include "../include/SSLSockProxyTCPClientSock.h"

namespace SockLib
{	
	CSSLSockProxyTCPClientSock::~CSSLSockProxyTCPClientSock()
	{
		if (m_pSSL_CTX) {
			SSL_CTX_free(m_pSSL_CTX);
			m_pSSL_CTX=NULL;
		}

		if (m_pSSL_HANDLE) {
			//SSL_shutdown(ssl_handler);
			SSL_free(m_pSSL_HANDLE);
			m_pSSL_HANDLE=NULL;
		}	

	}

	bool CSSLSockProxyTCPClientSock::Connect(std::string dest, unsigned short port, unsigned int timeout)
	{
		if(!m_bUseSSL) 
		{
			return CSockProxyTCPClientSock::Connect(dest,port,timeout);
		}
		else
		{
			printf("connect with ssl\n");
			SSL_library_init(); //always return 1
			return CSockProxyTCPClientSock::Connect(dest,port,timeout);
		}

	}


	bool CSSLSockProxyTCPClientSock::Connect(TInetAddr4 addr, unsigned int timeout)
	{
		if(!m_bUseSSL) 
		{
			return CSockProxyTCPClientSock::Connect(addr,timeout);
		}
		else
		{
			SSL_library_init(); //always return 1
			return CSockProxyTCPClientSock::Connect(addr,timeout);
		}

	}

	void CSSLSockProxyTCPClientSock::OnRead()
	{

		CSockProxyTCPClientSock::OnRead();
		if(CSockProxyTCPClientSock::CanRead())
		{
			if(!m_bUseSSL)
			{
				m_bSSLCanRead=true;
				return;
			}

			if(m_SSL_STATUS==S_CONNOK)
			{
				m_bSSLCanRead=true;
				return;
			}

			if(m_SSL_STATUS==S_CONN)
			{
				int nret=SSL_connect(m_pSSL_HANDLE);
				if(nret==0)
				{
					m_SSL_STATUS=S_CONNFAIL;
					RemoveTimer(m_SSL_Timer);
					OnConnectFail();				
				}
				else if(nret==1)
				{
					m_SSL_STATUS=S_CONNOK;
					RemoveTimer(m_SSL_Timer);
					OnConnectOk();
				}
				else
				{
					m_bSSLCanRead=false;
					int err=SSL_get_error(m_pSSL_HANDLE,nret);
					if(err!=SSL_ERROR_WANT_READ && err!=SSL_ERROR_WANT_WRITE)
					{
						printf("SSL_connect failed-6\n");
						m_SSL_STATUS=S_CONNFAIL;
						RemoveTimer(m_SSL_Timer);
						OnConnectFail();
					}
				}
			}
			else if(m_SSL_STATUS==S_CONNOK)
			{
				m_bSSLCanRead=true;
			}
		}
	}
	void CSSLSockProxyTCPClientSock::OnWrite()
	{

		CSockProxyTCPClientSock::OnWrite();
		if(CSockProxyTCPClientSock::CanWrite())
		{
			if(!m_bUseSSL) 
			{
				m_bSSLCanWrite=true;
				return;
			}

			if(m_SSL_STATUS==S_CONNOK)
			{
				m_bSSLCanWrite=true;
				return;
			}

			if(m_SSL_STATUS==S_CONN)
			{
				int nret=SSL_connect(m_pSSL_HANDLE);
				if(nret==0)
				{
					printf("SSL_connect failed-2\n");
					m_SSL_STATUS=S_CONNFAIL;
					RemoveTimer(m_SSL_Timer);
					OnConnectFail();				
				}
				else if(nret==1)
				{
					m_SSL_STATUS=S_CONNOK;
					RemoveTimer(m_SSL_Timer);
					OnConnectOk();
				}
				else
				{
					m_bSSLCanWrite=false;
					int err=SSL_get_error(m_pSSL_HANDLE,nret);
					if(err!=SSL_ERROR_WANT_READ && err!=SSL_ERROR_WANT_WRITE)
					{
						printf("SSL_connect failed-6\n");
						m_SSL_STATUS=S_CONNFAIL;
						RemoveTimer(m_SSL_Timer);
						OnConnectFail();
					}
				}
			}
			else if(m_SSL_STATUS==S_CONNOK)
			{
				m_bSSLCanWrite=true;
			}

		}
	}

	void CSSLSockProxyTCPClientSock::OnClose()
	{
		if (m_pSSL_CTX) {
			SSL_CTX_free(m_pSSL_CTX);
			m_pSSL_CTX=NULL;
		}

		if (m_pSSL_HANDLE) {
			//SSL_shutdown(ssl_handler);
			SSL_free(m_pSSL_HANDLE);
			m_pSSL_HANDLE=NULL;
		}	

		CSockProxyTCPClientSock::OnClose();
	}

	void CSSLSockProxyTCPClientSock::OnConnectFail()
	{

		if (m_pSSL_CTX) {
			SSL_CTX_free(m_pSSL_CTX);
			m_pSSL_CTX=NULL;
		}

		if (m_pSSL_HANDLE) {
			//SSL_shutdown(ssl_handler);
			SSL_free(m_pSSL_HANDLE);
			m_pSSL_HANDLE=NULL;
		}

		CSockProxyTCPClientSock::OnConnectFail();

	}

	void CSSLSockProxyTCPClientSock::OnConnectOk()
	{
		CSockProxyTCPClientSock::OnConnectOk();
		if(CSockProxyTCPClientSock::CanWrite())
		{
			if(!m_bUseSSL)
			{
				m_bSSLCanWrite=true;
				return;
			}

			//make our SSL_Connect if m_SSL_STATUS!=S_CONNOK;
			//if(m_SSL_STATUS==S_CONNOK) return;
			if(m_SSL_STATUS==S_CONNOK)
			{
				m_bSSLCanWrite=true;
				return;
			}

			assert(m_SSL_STATUS==S_INIT);

			printf("start our ssl_connect\n");

			m_pSSL_CTX = SSL_CTX_new(SSLv23_client_method());
			m_pSSL_HANDLE = SSL_new(m_pSSL_CTX);

			if(1!=SSL_set_fd(m_pSSL_HANDLE,GetHandle()))
			{
				printf("SSL_set_fd failed\n");
				m_SSL_STATUS=S_CONNFAIL;
				OnConnectFail();
				return;
			}

			int nret=SSL_connect(m_pSSL_HANDLE);
			if (nret==0)
			{
				printf("SSL_connect failed-1\n");
				m_SSL_STATUS=S_CONNFAIL;
				OnConnectFail();
				return;
			}	
			else if(nret<0)
			{
				printf("ssl_connect return=%d\n",nret);

				int err=SSL_get_error(m_pSSL_HANDLE,nret);
				if(err!=SSL_ERROR_WANT_READ && err!=SSL_ERROR_WANT_WRITE)
				{
					printf("SSL_connect failed-5\n");
					m_SSL_STATUS=S_CONNFAIL;
					OnConnectFail();
					return;
				}
			}

			m_SSL_STATUS=S_CONN;
			maskRead(true);
			maskWrite(true);
			m_SSL_Timer=AddTimer(m_nTimeOut,true);
		}
	}

	int CSSLSockProxyTCPClientSock::Send(char* buf, int len)
	{
		if(!m_bUseSSL) return CSockProxyTCPClientSock::Send(buf,len);

		//assert(m_pSSL_HANDLE);
		//assert(m_SSL_STATUS==S_CONNOK);	
		if(m_SSL_STATUS!=S_CONNOK) return CSockProxyTCPClientSock::Send(buf,len);
		else return SSL_write(m_pSSL_HANDLE,buf,len);
	}

	int CSSLSockProxyTCPClientSock::Recv(char* buf, int len)
	{
		if(!m_bUseSSL) return CSockProxyTCPClientSock::Recv(buf,len);

		if(m_SSL_STATUS!=S_CONNOK) return CSockProxyTCPClientSock::Recv(buf,len);
		else return SSL_read(m_pSSL_HANDLE,buf,len);
		//assert(m_pSSL_HANDLE);
		//assert(m_SSL_STATUS==S_CONNOK);
		//return SSL_read(m_pSSL_HANDLE,buf,len);
	}

	void CSSLSockProxyTCPClientSock::OnTimer(unsigned int id)
	{
		CSockProxyTCPClientSock::OnTimer(id);
		if(id==m_SSL_Timer)
		{
			m_SSL_Timer=0;
			if(m_SSL_STATUS!=S_CONNOK)	OnConnectFail();
		}
	}

}
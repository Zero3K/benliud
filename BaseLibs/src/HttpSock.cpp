/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#include "../include/HttpSock.h"

namespace SockLib
{	
	void CHttpSock::OnRead()
	{
		CSSLSockProxyTCPClientSock::OnRead();
		if(!CanRead()) return;

		char buffer[2048];



		while(1)
		{
			int nret=Recv(buffer,2048);
			if(nret==0)
			{
				OnClose();
				return;
			}
			else if(nret<0)
			{
#ifdef WIN32

				if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

				if ( errno == EAGAIN || errno == EWOULDBLOCK )
#endif
				{
					return;
				}
				else
				{
					OnClose();
					return;
				}
			}
			else
			{
				m_pAcceptor->DataNotice(this,nret,buffer);
			}
		}
	}

	void CHttpSock::OnWrite()
	{
		CSSLSockProxyTCPClientSock::OnWrite();
		if(!CanWrite()) return;

		if(m_sSendBuf.empty())
		{
			maskWrite(false);
			return;
		}
		else
		{
			//send out our data, if have error close socket
			//if buffer can't hold so much bytes, mask write again
			//else if send out all data, cancel mask write

			maskWrite(false); //assume all data can be written out

			while(!m_sSendBuf.empty())
			{
				int nret=Send((char*)m_sSendBuf.data(),m_sSendBuf.size());
				if(nret==0)
				{
					OnClose();
					return;
				}
				else if(nret<0)
				{
#ifdef WIN32

					if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

					if ( errno == EAGAIN || errno == EWOULDBLOCK )
#endif
					{
						maskWrite(true);	//too much bytes can't write in buffer.
						return;
					}
					else
					{
						OnClose();
						return;
					}
				}
				else
				{
					m_sSendBuf.erase(0,nret);
				}
			}
		}
	}

	void CHttpSock::OnClose()
	{
		CSSLSockProxyTCPClientSock::OnClose();
		m_pAcceptor->CloseNotice(this);
	}

	void CHttpSock::OnConnectFail()
	{
		CSSLSockProxyTCPClientSock::OnConnectFail();
		m_pAcceptor->FailNotice(this);
	}

	void CHttpSock::OnConnectOk()
	{
		CSSLSockProxyTCPClientSock::OnConnectOk();
		m_pAcceptor->OpenNotice(this);
	}


}
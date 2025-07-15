/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _HTTPSOCK_H_
#define _HTTPSOCK_H_

#include "SSLSockProxyTCPClientSock.h"
#include "StreamDataAcceptor.h"

namespace SockLib
{

	enum HTTPLINK_STATUS
	{
		HTTPLINK_INIT, 
		HTTPLINK_CONN, 
		HTTPLINK_CONNOK, 
		HTTPLINK_FAIL, 
		HTTPLINK_CLOSE,
	};

	class CHttpSock: public CSSLSockProxyTCPClientSock 
	{
	public:
		CHttpSock(CStreamDataAcceptor* acceptor)
		{
			m_pAcceptor=acceptor;
			m_HttpLinkStatus=HTTPLINK_INIT;
		}

		void SetDestUrl(std::string url)
		{
			m_sDestUrl=url;
		}

		void SendData(std::string& content)
		{
			m_sSendBuf+=content;
			maskWrite(true);
		}

		virtual void OnRead();
		virtual void OnWrite();
		virtual void OnClose();
		virtual void OnConnectFail();
		virtual void OnConnectOk();

		void Close() 
		{
			OnClose();
		}

	protected:
		HTTPLINK_STATUS m_HttpLinkStatus;
		CStreamDataAcceptor* m_pAcceptor;
		std::string m_sDestUrl;
		std::string m_sSendBuf;	//send out content buffer
	private:

	};
}
#endif // _HTTPSOCK_H_

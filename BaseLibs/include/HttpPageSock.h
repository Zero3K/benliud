/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _HTTPPAGESOCK_H_
#define _HTTPPAGESOCK_H_
#include "SSLSockProxyTCPClientSock.h"

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

	class CHttpPageSock: public CSSLSockProxyTCPClientSock 
	{
	public:
		CHttpPageSock()
		{

		}

		void SetDestUrl(std::string url)
		{

		}

		virtual void OnRead();
		virtual void OnWrite();
		virtual void OnClose();
		virtual void OnConnectFail();
		virtual void OnConnectOk();

		void Go();

	protected:

	private:

	};
}
#endif // _HTTPPAGESOCK_H_

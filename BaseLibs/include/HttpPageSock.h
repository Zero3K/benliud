/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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

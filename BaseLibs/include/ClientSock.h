/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

#ifndef _CLIENTSOCK_H_
#define _CLIENTSOCK_H_

#include "Sock.h"
namespace SockLib
{
	class CClientSock: public CSock 
	{
	public:
		CClientSock():m_nTimeOut(20000){}
		virtual bool Connect(TInetAddr4 addr, unsigned int timeout)
		{
			return true;
		}; //if have proxy ,udp also need connect with proxy.
		virtual bool Connect(std::string dest, unsigned short port, unsigned int timeout)
		{
			return true;
		};

		virtual void OnClose()
		{
			Close();
		}
	protected:
		unsigned int m_nTimeOut; //for connect 
	private:

	};
}
#endif // _CLIENTSOCK_H_

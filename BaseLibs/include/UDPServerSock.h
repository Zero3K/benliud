/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

#ifndef _UDPSERVERSOCK_H_
#define _UDPSERVERSOCK_H_

#include "ServerSock.h"
#include "Dealer.h"
namespace SockLib
{
	class CUDPServerSock: public CServerSock 
	{
	public:
		bool CreateSock()
		{
			assert(m_hSocket==-1);
			m_hSocket = socket( AF_INET, SOCK_DGRAM,0 );

			if(m_hSocket <=0) return false;

			SetBlock(false);
			maskRead(false);
			maskWrite(false);
			if(m_pDealer!=NULL) m_pDealer->AddSockClient(this);
			return true;	
		}
	protected:

	private:

	};
}
#endif // _UDPSERVERSOCK_H_

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _SOCKDEALER_H
#define _SOCKDEALER_H

#include <list>
#include <vector>
#ifdef WIN32
#include <winsock2.h>
#endif
#include "Mutex.h"




namespace SockLib
{
	class CSock;
	typedef std::list<CSock*> TSocketList;
	class CSockDealer  
	{
	public:
		void DispatchEvent();
		virtual void RemoveSockClient(CSock* client);
		void AddSockClient(CSock* client);
		CSockDealer();
		virtual ~CSockDealer();

	protected:
		void CleanClient();
		TSocketList m_SocketList;

		CMutex		m_RemoveMutex;
		CMutex		m_PendingListMutex;
		TSocketList m_PendingList;
	};
}
#endif 

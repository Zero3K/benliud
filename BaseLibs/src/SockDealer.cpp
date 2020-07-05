/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <algorithm>

#include "../include/SockDealer.h"
#include "../include/Sock.h"
#include "../include/AutoLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace SockLib
{	
	CSockDealer::CSockDealer()
	{

	}

	CSockDealer::~CSockDealer()
	{

	}


	void CSockDealer::AddSockClient(CSock *client)
	{

		//CAutoLock al(m_PendingListMutex);
		assert(client!=NULL);

		m_PendingList.push_back(client);



	}

	void CSockDealer::RemoveSockClient(CSock *client)
	{

		//CAutoLock al(m_PendingListMutex);

		TSocketList::iterator it;

		for ( it = m_SocketList.begin();it != m_SocketList.end();it++ )
		{
			if ( *it == client )
			{
				CAutoLock al(m_RemoveMutex);
				(*it) = NULL;	//just mark off it, don't erase it.
				return;
			}
		}

		m_PendingList.remove(client);

	}

	void CSockDealer::DispatchEvent()
	{
		struct timeval timeout;  // Timeout for select
		fd_set rsset, wsset;
		int havesock = 0;
		TSocketList::const_iterator it;

		FD_ZERO( &rsset );
		FD_ZERO( &wsset );

		//build socket set and use select to wait event


		//CAutoLock al(m_PendingListMutex);

		CAutoLock al(m_RemoveMutex);

		CleanClient();


		for ( it = m_SocketList.begin();it != m_SocketList.end();it++)
		{

			if ( /*(*it)!=NULL &&*/ ( *it ) ->maskRead() )
			{
				FD_SET( ( *it ) ->GetHandle(), &rsset );
				havesock++;
			}

			if ( /*(*it)!=NULL &&*/ ( *it ) ->maskWrite() )
			{
				FD_SET( ( *it ) ->GetHandle(), &wsset );
				havesock++;
			}

		}


		if ( !havesock )
		{
#ifdef WIN32
			Sleep( 30 );
#else
			usleep(30*1000);
#endif
			return ;
		}


		timeout.tv_sec = 0;
		timeout.tv_usec = 1000 * 30; //30 ms wait

		//check if socket can read/write
		int n = select( FD_SETSIZE, &rsset, &wsset, ( fd_set* ) 0, &timeout );

		if ( n > 0 )
		{
			//some socket can read/write
			//find it out
			//CAutoLock al(m_PendingListMutex);

			TSocketList templist;

			for ( it = m_SocketList.begin();it != m_SocketList.end();it++ )
			{
				if ( /*(*it)!=NULL &&*/( *it ) ->maskRead() && FD_ISSET( ( *it ) ->GetHandle(), &rsset ) )
				{
					templist.push_back(*it);
				}
			}

			for ( it = templist.begin();it != templist.end();it++ )
			{
				assert((*it)!=NULL);
				(*it)->OnRead();  //OnRead() maybe delete itself in SocketList
			}

			templist.clear();

			CleanClient(); //OnRead() maybe delete itself in SocketList

			for ( it = m_SocketList.begin();it != m_SocketList.end();it++ )
			{
				if ( /*(*it)!=NULL &&*/ ( *it ) ->maskWrite() && FD_ISSET( ( *it ) ->GetHandle(), &wsset ) )
				{
					templist.push_back(*it);
				}
			}

			for ( it = templist.begin();it != templist.end();it++ )
			{
				assert((*it)!=NULL);
				(*it)->OnWrite();  //because OnWrite() maybe delete itself in SocketList
			}

			templist.clear();



		} //if n>0
		else if(n<0)
		{
			printf("select error\n");
#ifdef WIN32
			Sleep(10); //select error
#else
			usleep(10*1000);
#endif
		}
		else
		{
			//Check();
		}
		//	else if (n==0)
		//	{
		//printf("select timeout\n");
		//	}
		//	else// if(n==SOCKET_ERROR)
		//	{
		//		printf("select error\n");
		//	}

	}

	void CSockDealer::CleanClient()
	{
		TSocketList::const_iterator it;

		m_SocketList.remove(NULL);

		//CAutoLock al(m_PendingListMutex);

		for ( it = m_PendingList.begin(); it != m_PendingList.end(); ++it )
		{
			m_SocketList.push_back( *it );
		}

		m_PendingList.clear();

	}

}
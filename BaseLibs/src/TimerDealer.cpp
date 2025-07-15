/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#include "../include/TimerDealer.h"
#include "../include/TimerClient.h"
#include "../include/Sock.h"
#include "../include/AutoLock.h"
#include "../include/Tools.h"


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <assert.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace SockLib
{
	CTimerDealer::CTimerDealer()
	{
		m_nTimerCounter=0;
	}

	CTimerDealer::~CTimerDealer()
	{

	}

	unsigned int CTimerDealer::AddTimer(CTimerClient *client, unsigned int interval, bool oneshot)
	{

		CAutoLock al(m_PendingListMutex);
		assert(client!=NULL);

		TTimerInfo timerInfo;

		timerInfo.id = ++m_nTimerCounter; //这里出现过崩溃问题，可能是连接到不一致的库造成的

		timerInfo.interval = interval;
		timerInfo.lastshot = GetTickCount();
		timerInfo.client = client;
		timerInfo.oneshot = oneshot;

		m_PendingList.push_back( timerInfo );

		return timerInfo.id;
	}

	void CTimerDealer::RemoveTimer(unsigned int id)
	{
		TTimerList::iterator it;
		CAutoLock al(m_PendingListMutex);

		for ( it = m_TimerList.begin(); it != m_TimerList.end(); it++ )
		{
			if ( it->id == id )
			{
				it->client = NULL;
				return; //or break??
			}
		}



		for ( it = m_PendingList.begin(); it != m_PendingList.end(); it++ )
		{
			if ( it->id == id )
			{
				m_PendingList.erase( it );
				break;
			}
		}

	}

	void CTimerDealer::DispatchTimer()
	{

		CleanClient();

		TTimerList::iterator it;


		for ( it = m_TimerList.begin(); it != m_TimerList.end(); )
		{

			if ( (it->client!=NULL) && ( GetTickCount() >= it->lastshot + it->interval) )
			{

				assert(it->client!=NULL);

				it->client->OnTimer( it->id );

				it->lastshot = GetTickCount();

				if(it->oneshot)
				{
					//it = m_TimerList.erase(it);			
					m_TimerList.erase(it++); //for vc8 test
					continue;
				}
			}

			it++;
		}



	}

	void CTimerDealer::RemoveTimerClient(CTimerClient *client)
	{
		CAutoLock al(m_PendingListMutex);

		TTimerList::iterator it;

		for ( it = m_TimerList.begin(); it != m_TimerList.end(); it++ )
		{
			if ( it->client == client )
			{
				it->client = NULL;
			}
		}


		for ( it = m_PendingList.begin(); it != m_PendingList.end(); /*it++*/ )
		{
			if ( it->client == client )
			{
				m_PendingList.erase(it++); //for vc8 test
				continue;
			}

			it++;
		}	


	}

	void CTimerDealer::CleanClient()
	{
		TTimerList::iterator it;
		CAutoLock al(m_PendingListMutex);	

		for ( it = m_TimerList.begin(); it != m_TimerList.end(); )
		{
			if ( it->client == NULL )
			{
				m_TimerList.erase(it++); //for vc8 test
				continue;
			}

			it++;
		}




		for ( it = m_PendingList.begin(); it != m_PendingList.end(); ++it )
		{
			//if( it->client !=NULL )   m_TimerList.push_back( *it );
			m_TimerList.push_back( *it );
		}

		m_PendingList.clear();

	}
}
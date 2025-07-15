/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// BTListenSpeedControl.cpp: implementation of the CBTListenSpeedControl class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/BTListenSpeedControl.h"
#include "../include/BTPeer.h"
#include <AutoLock.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void syslog(std::string s);

CBTListenSpeedControl::CBTListenSpeedControl()
{

}

CBTListenSpeedControl::~CBTListenSpeedControl()
{

}

void CBTListenSpeedControl::Update()
{
	CleanClient();
	Download();
	Upload();
}

void CBTListenSpeedControl::RegisteClient(CBTPeer* client)
{
	SockLib::CAutoLock al(m_PendingListMutex);
	//m_PendingListMutex.Lock();
	m_PendingList.push_back(client);
	//m_PendingListMutex.Unlock();

    //m_ClientList.push_back( client );
}

void CBTListenSpeedControl::UnregisteClient(CBTPeer* client)
{
    //make remove mark, remove in update
    TClientList::iterator it;

    for ( it = m_ClientList.begin(); it != m_ClientList.end(); it++ )
    {
        if ( ( *it ) == client )
        {
            ( *it ) = NULL;
            return;
        }
    }


	SockLib::CAutoLock al(m_PendingListMutex);

	//m_PendingListMutex.Lock();

	m_PendingList.remove(client);

	//m_PendingListMutex.Unlock();

}

void CBTListenSpeedControl::CleanClient()
{
	/*
    TClientList::iterator it ;

    for ( it = m_ClientList.begin(); it != m_ClientList.end(); )
    {
        if ( *it == NULL )
            it = m_ClientList.erase( it );
        else
            it++;
    }
	*/


 

	m_ClientList.remove(NULL);

	//m_PendingListMutex.Lock();
	SockLib::CAutoLock al(m_PendingListMutex);

    TClientList::iterator it;

    for ( it = m_PendingList.begin(); it != m_PendingList.end(); ++it )
    {
		m_ClientList.push_back( *it );
    }

    m_PendingList.clear();

	//m_PendingListMutex.Unlock();


}

void CBTListenSpeedControl::Download()
{
    TClientList::iterator it ;

    for ( it = m_ClientList.begin(); it != m_ClientList.end(); it++ )
    {

        if ( *it )
        {
			//在这个处理过程中，如果对方发起加密连接
			//最后的IA中携带了握手包和bitset，则本监听线程
			//会进入bitset处理过程，在这个过程中发生的交叉
			//会进入到Storage等很多本不应该进入的区域，虽然
			//目前看代码来说，进入到这些区域看起来没有问题，
			//但是，确实曾经捕捉到因为这个线程向Storage提交
			//bitset变化而发生的崩溃！所以如果对方IA中带有bitset
			//我们现在直接关闭这个连接
			(*it)->DoRead(0x0FFFFFFF); //无速度限制

        }

    }

}

void CBTListenSpeedControl::Upload()
{
    TClientList::iterator it;

	//command send
    for ( it = m_ClientList.begin(); it != m_ClientList.end();it++ )
    {
        if ( *it )
        {
			(*it)->DoCmdWrite(0x0FFFFFFF,true); //no speed limit for command
        }
    }
}

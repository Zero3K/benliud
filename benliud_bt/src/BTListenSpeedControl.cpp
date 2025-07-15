/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

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
			//��������������У�����Է������������
			//����IA��Я�������ְ���bitset���򱾼����߳�
			//�����bitset�������̣�����������з����Ľ���
			//����뵽Storage�Ⱥܶ౾��Ӧ�ý����������Ȼ
			//Ŀǰ��������˵�����뵽��Щ��������û�����⣬
			//���ǣ�ȷʵ������׽����Ϊ����߳���Storage�ύ
			//bitset�仯�������ı�������������Է�IA�д���bitset
			//��������ֱ�ӹر��������
			(*it)->DoRead(0x0FFFFFFF); //���ٶ�����

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

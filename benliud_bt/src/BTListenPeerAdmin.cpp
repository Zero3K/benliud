/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

// BTListenPeerAdmin.cpp: implementation of the CBTListenPeerAdmin class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/BTListenPeerAdmin.h"
#include "../include/BTPeer.h"
#include <Dealer.h>

#include <assert.h>

extern void syslog( std::string info );
extern void logsysmem(int id);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBTListenPeerAdmin::CBTListenPeerAdmin(CBTListener* parent)
{
	m_pParent=parent;
	m_nCheckTimer=0;
}

CBTListenPeerAdmin::~CBTListenPeerAdmin()
{

}


SockLib::CDealer* CBTListenPeerAdmin::GetDealer()
{
	return m_pParent->GetDealer();
}

CSpeedControlBase* CBTListenPeerAdmin::GetSpeedControl()
{
	return m_pParent->GetSpeedControl();
}

CBTSession*	CBTListenPeerAdmin::GetSession()
{
	return NULL;
}

//�����յ����ӻ��infohashʱ��������������Ժ󼴿����ж��ǹرջ���ת���������
bool CBTListenPeerAdmin::GotHash(std::string hash, CBTPeer* client)
{

	bool bret=m_pParent->LinkGotHash(hash,client);
	
	//�����棬���ӱ�ת��
	if(bret)
	{

		//ɾ�����������Ŀ���������ǹ���
		/*
		TPeerInfoMap::iterator it;

		for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++)
		{

			if ( it->second.peerLink == client )
			{
				m_ConnectedPeerList.erase( it );  
				break;
			}
		}*/

		m_ConnectedPeerList.remove(client);

		return true; //ԭ����Ӧ��������
	}
	else
	{
		//���ؼ٣�������Ҫ�ر�
		client->ClosePeer();	//�����ӵ�
		return false;//�����Լ��أ��������״�

	}

}

void CBTListenPeerAdmin::OnTimer(unsigned int id)
{
	//check and clean the closed peer
	if(m_nCheckTimer!=id) return;
	CheckClosedConnection();  //�����Ѿ��رյ�����
}


void CBTListenPeerAdmin::CheckClosedConnection()
{
	//��������Ѿ��رյ����Ӳ�������Դ

	TPeerList::iterator it;

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); )
    {

        if ( (*it)->GetLinkState() == LS_CLOSE )
        {
            delete (*it);
			//it=m_ConnectedPeerList.erase(it);
			m_ConnectedPeerList.erase(it++); //for vc8 test
            continue;
        }

        it++;
    }

}

void CBTListenPeerAdmin::Start()
{
	m_nCheckTimer=m_pParent->GetDealer()->AddTimer(this, 2000);
}

void CBTListenPeerAdmin::Stop()
{
	m_pParent->GetDealer()->RemoveTimerClient(this);
	m_nCheckTimer=0;

	//�ر���������
	TPeerList::iterator it;

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++)
    {
        (*it)->ClosePeer();
        delete (*it);
    }

	m_ConnectedPeerList.clear();
}

bool CBTListenPeerAdmin::GotEncryptHash(std::string hashxor, MSE::BigInt dhsecret, CBTPeer* client)
{

	bool bret=m_pParent->LinkGotEncryptHash(hashxor,dhsecret,client);
	
	//�����棬���ӱ�ת��
	if(bret)
	{
		//ɾ�����������Ŀ���������ǹ���
		m_ConnectedPeerList.remove(client);

		return true; //ԭ����Ӧ��������
	}
	else
	{
		//���ؼ٣�������Ҫ�ر�

		return false;//�����Լ��أ��������״�

	}

}

bool CBTListenPeerAdmin::NewAccept(int handle, unsigned int iip, unsigned short iport)
{

	TPeerList::const_iterator it;
	for(it=m_ConnectedPeerList.begin();it!=m_ConnectedPeerList.end();it++)
	{
		if((*it)->GetPeeriIP()==iip) return false;  //�����ܶԷ�ͬʱ�������������
	}


    CBTPeer* peerLink = new CBTPeer(this); //���ɼ����߹��������infohash����ת��


	peerLink->SetDealer(m_pParent->GetDealer());

    peerLink->Attach( handle, iip, iport ); //port here is only for show message
	
	//Ӧ�ò���Ҫ����

    m_ConnectedPeerList.push_back(peerLink);  

	return true;
}

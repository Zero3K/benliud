/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


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

//当接收的连接获得infohash时调用这个，完了以后即可以判断是关闭还是转移这个连接
bool CBTListenPeerAdmin::GotHash(std::string hash, CBTPeer* client)
{

	bool bret=m_pParent->LinkGotHash(hash,client);
	
	//返回真，连接被转移
	if(bret)
	{

		//删除这个连接条目，不归我们管了
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

		return true; //原连接应继续运行
	}
	else
	{
		//返回假，连接需要关闭
		client->ClosePeer();	//新增加的
		return false;//让他自己关，否则容易错

	}

}

void CBTListenPeerAdmin::OnTimer(unsigned int id)
{
	//check and clean the closed peer
	if(m_nCheckTimer!=id) return;
	CheckClosedConnection();  //清理已经关闭的连接
}


void CBTListenPeerAdmin::CheckClosedConnection()
{
	//负责监视已经关闭的连接并回收资源

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

	//关闭所有连接
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
	
	//返回真，连接被转移
	if(bret)
	{
		//删除这个连接条目，不归我们管了
		m_ConnectedPeerList.remove(client);

		return true; //原连接应继续运行
	}
	else
	{
		//返回假，连接需要关闭

		return false;//让他自己关，否则容易错

	}

}

bool CBTListenPeerAdmin::NewAccept(int handle, unsigned int iip, unsigned short iport)
{

	TPeerList::const_iterator it;
	for(it=m_ConnectedPeerList.begin();it!=m_ConnectedPeerList.end();it++)
	{
		if((*it)->GetPeeriIP()==iip) return false;  //不接受对方同时发起的两个连接
	}


    CBTPeer* peerLink = new CBTPeer(this); //先由监听者管理，获得infohash后再转移


	peerLink->SetDealer(m_pParent->GetDealer());

    peerLink->Attach( handle, iip, iport ); //port here is only for show message
	
	//应该不需要枷锁

    m_ConnectedPeerList.push_back(peerLink);  

	return true;
}

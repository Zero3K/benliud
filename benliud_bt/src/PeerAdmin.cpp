/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/

#include "stdafx.h"

#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/PeerAdmin.h"
#include "../include/BTPeer.h"
#include "../include/BTSession.h" 
#include <TorrentFile.h>
#include <Tools.h>
#include <Dealer.h>
#include <AutoLock.h>
#include "../include/BTPiece.h"
#include "../include/BTStorage.h"
#include "../include/BTJob.h"
#include "../include/SpeedControl.h"
#include <assert.h>

#include <queue>
#include <vector>
//interval of check connection in second
//#define CHECKCONN (1)
//#define KEEPLINK  (12)  //ٱЧ

extern void syslog(std::string info);
extern void logsysmem(int id);

class _DownloadCompare
{
public:
  bool operator() (  CBTPeer*& lclient,  CBTPeer*& rclient ) 
  {
    return (lclient->GetDownloadPriority() > rclient->GetDownloadPriority()); //check >= for vc8
  }
};

class _UploadCompare
{
public:
  bool operator() (  CBTPeer*& lclient,  CBTPeer*& rclient ) 
  {
    return (lclient->GetUploadPriority() > rclient->GetUploadPriority());
  }
};

CPeerAdmin::CPeerAdmin( CBTSession* sess , bool wefinish)
{
    m_pSession = sess;
	m_ConnectCheckTimer=0;
	m_ChokeCheckTimer=0;
	m_nTotalPeers=0;
	m_bDownloadFinish=wefinish;
	m_EncryptionMode=_PREFER_ORDINARY;
	m_bSwitchMode=false;

}

CPeerAdmin::~CPeerAdmin()
{

}


CBTSession* CPeerAdmin::GetSession()
{
    return m_pSession;
}

bool CPeerAdmin::Start()
{

    m_ConnectCheckTimer = m_pSession->GetDealer() ->AddTimer( this, 1000 );

    m_ChokeCheckTimer = m_pSession->GetDealer() ->AddTimer( this, 5000 ); 

    return true;
}

void CPeerAdmin::Stop()
{

    m_pSession->GetDealer() ->RemoveTimer( m_ConnectCheckTimer );
    m_pSession->GetDealer() ->RemoveTimer( m_ChokeCheckTimer );

	TPeerList::iterator it;

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
    {

		assert(*it);
#ifdef _CHECK
		CBTPeer* peerLink=(*it);
		bool bencrypt	=peerLink->IsEncrypt();
        bool accepted	=peerLink->IsAccepted();

		//TCloseReason cr	=peerLink->GetCloseReason();
		std::string pid	=peerLink->GetPeerId();
		unsigned int iip=peerLink->GetPeeriIP();

		if(peerLink->IsGotBitSet())
		{
			CBTPiece bitset=peerLink->GetPeerBitSet();
			m_pSession->CloseReport(iip,CR_MYCLOSE,accepted,pid,&bitset);
		}
		else
		{
			m_pSession->CloseReport(iip,CR_MYCLOSE,accepted,pid,NULL);
		}
#endif
		(*it)->JobEndClose();
		delete (*it);
    }

    for ( it = m_ConnectingPeerList.begin();it != m_ConnectingPeerList.end(); it++ )
    {
		assert(*it);
		(*it)->JobEndClose();
		delete (*it);
    }

}


void CPeerAdmin::OnTimer( unsigned int id )
{

    if ( id == m_ConnectCheckTimer )
    {
        CheckPeerConnection();
    }
    else if ( id == m_ChokeCheckTimer )
    {
        AdjustChoke();
		//Debug();
    }

}



//class _UnChokeCompare
//{
//public:
//  bool operator() (  CBTPeer*& lclient,  CBTPeer*& rclient ) 
//  {
//    return (lclient->GetUploadPriority() > rclient->GetUploadPriority());
//  }
//};


void CPeerAdmin::AdjustChoke()
{
	SockLib::CAutoLock al(m_ConnectedPeerListMutex);
	//in PriorityQueue small value is on top like 1,2,3...
	//std::priority_queue<CBTPeer*, std::vector<CBTPeer*>, _UnChokeCompare> PriorityQueue; //#include <queue>
	std::priority_queue<CBTPeer*, std::vector<CBTPeer*>, _UploadCompare> PriorityQueue;

	//unchoke client number limit
	unsigned int nNum=m_pSession->GetUploadLinkMax();

#ifdef _CHECK
//	wchar_t msg[128];
//	swprintf(msg,L"AdjustChoke, upload slot=%d, link=%d",nNum,m_ConnectedPeerList.size());
//	OutMsg(msg);
#endif

	assert(nNum>0 && nNum < 1000);
	
	TPeerList::iterator it;


    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
    {
        CBTPeer* peerLink = (*it);
		assert(peerLink);

		if( peerLink->PeerInterestMe() )
		{//the peer not interested me ignored!
			//so the peer will have more chance to get our upload.
			if(PriorityQueue.size() < nNum)
			{
				PriorityQueue.push( peerLink );
			}
			else if(peerLink->GetUploadPriority() > PriorityQueue.top()->GetUploadPriority())
			{
				PriorityQueue.top()->ChokePeer(true);  //all pop out should be choked
				PriorityQueue.pop();		//pop the smallest value
				PriorityQueue.push(peerLink); //push this large value
			}
		}
		else
		{
			peerLink->ChokePeer(true);	//peer not interest me
		}

    }

	//do the unchoke on PriorityQueue
	while(!PriorityQueue.empty())
	{
		PriorityQueue.top()->ChokePeer(false);	//all left in queue should be unchoked
		PriorityQueue.pop();
	}


}


void CPeerAdmin::CheckPeerConnection()
{
	if(m_bSwitchMode) //finish to not finish, or not finish to finish mark
	{
		SwitchMode();
	}

	if(!m_bDownloadFinish) 
	{
		CheckPeerConnectionWhenNotFinish();
	}
	else 
	{
		CheckPeerConnectionWhenFinished();
	}
}


void CPeerAdmin::BroadcastNewPiece( int index )
{

	TPeerList::iterator it;

	SockLib::CAutoLock al(m_ConnectedPeerListMutex);

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
    {
		CBTPeer* peerLink=(*it);
		assert(peerLink);
        peerLink->BroadcastNewPiece( index );

    }

}


//storage call to notice we are finished or not finish
void CPeerAdmin::DownloadFinish(bool finish)
{
	if(m_bDownloadFinish!=finish)
	{
		m_bSwitchMode=true;
		m_bDownloadFinish=finish;
	}

}

//DEL void CPeerAdmin::PieceChange( CBTPiece& peer, bool newpeer )
//DEL {//directly send to storage
//DEL     m_pSession->GetStorage() ->PieceChangeNotice( peer, newpeer );
//DEL }


void CPeerAdmin::LaunchNewConnectionWhenNotFinish()
{

	unsigned int iip;
	unsigned short iport;
	int encref;
	unsigned int timeout;

	//ԽնٸЧӵĿλ
	int empty= m_pSession->GetLinkMax() - int(m_ConnectedPeerList.size());

	//part 1: ûбͣѡȷӵ
	//˶ԷҪãܱԷΪǸŶôһβҪѡ̫Ŀ
	//׵ûпռɶӡ

	int limit = (empty<=0) ? 3 : empty+3; //ƶԿӵĽڵ㷢̫ӣݲ

	while(m_pSession->GetPeerInfoToLink(true, iip,iport,encref,timeout))
	{
	
		if(--limit < 0)
		{
			m_pSession->GiveUpLink(iip);
			break;
		}


		CBTPeer* peerLink = new CBTPeer(this);


		/*/
		switch(m_EncryptionMode)
		{
		case _ALWAYS_ENCRYPT:
			peerLink->SetEncrypt(true,true);
			break;
		case _ALWAYS_ORDINARY:
			break;
		case _PREFER_ENCRYPT:
			if(encref%2) 
			{//encryption have fail one time, try normal connect
				//dont set encrypt
			}	
			else
			{//not 
				peerLink->SetEncrypt(true,true); //if this conn fail ,set the EncryptRef mark
			}
			break;
		case _PREFER_ORDINARY:
		default:
			if(encref%2)
			{
				peerLink->SetEncrypt(true,true);
			}	
			break;
		}
		/*/

		if(encref%2) 
		{//Ǽ
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
		else
		{//˫Ϊ
			peerLink->SetEncrypt(true,true);
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}

		//*/
		
		if(peerLink->CreateSock())
		{
			peerLink->SetDealer(GetDealer());
			peerLink->Connect( iip, iport ,timeout);
		
			m_ConnectingPeerList.push_back(peerLink);
		}
		else
		{
			delete peerLink;
			m_pSession->GiveUpLink(iip);
		}

	}

	if(limit <= 0)
	{
		return;
	}

	//part 2, ѡûȷ״̬Ľڵ,δУĺУʧܵ
	while(m_pSession->GetPeerInfoToLink(false, iip,iport,encref,timeout))
	{
		if(--limit < 0)
		{
			m_pSession->GiveUpLink(iip);
			break;
		}
		

		CBTPeer* peerLink = new CBTPeer(this);


/*/
		switch(m_EncryptionMode)
		{
		case _ALWAYS_ENCRYPT:
			peerLink->SetEncrypt(true,true);
			break;
		case _ALWAYS_ORDINARY:
			break;
		case _PREFER_ENCRYPT:
			if(encref%2) 
			{//encryption have fail one time, try normal connect
				//dont set encrypt
			}	
			else
			{//not 
				peerLink->SetEncrypt(true,true); //if this conn fail ,set the EncryptRef mark
			}
			break;
		case _PREFER_ORDINARY:
		default:
			if(encref%2)
			{
				peerLink->SetEncrypt(true,true);
			}	
			break;
		}
/*/
		if(encref%2) 
		{//Ǽ
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
		else
		{//˫Ϊ
			peerLink->SetEncrypt(true,true);
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}

//*/
		if(peerLink->CreateSock())
		{
			peerLink->SetDealer(GetDealer());
			peerLink->Connect( iip, iport ,timeout);
		
			m_ConnectingPeerList.push_back(peerLink);
		}
		else
		{
			delete peerLink;
			m_pSession->GiveUpLink(iip);
		}

	}

}

void CPeerAdmin::CheckPeerConnectionWhenNotFinish()
{

    // move new connection to m_ConnectedPeerList
    CheckConnectedConnecting();

	//move the closed connecting to unusedlist
	CheckClosedConnecting();

	//try to lanch new peers if have space
	LaunchNewConnectionWhenNotFinish();	

	//close the timeout connections
	CloseTimeoutConnection();

    //remove the closed connection
	CheckClosedConnection();
}

void CPeerAdmin::CheckPeerConnectionWhenFinished()
{
	//upload mode , pick the most least data peer to upload?
    // move new connection to m_ConnectedPeerList
    CheckConnectedConnecting();
	
	//move the closed connecting to unusedlist
	CheckClosedConnecting();

    //move closed in connected list to m_UnusedPeerList;
	CheckClosedConnection();

	//make new conn
	LaunchNewConnectionWhenFinished(); 
}



int CPeerAdmin::CloseInterestedPeer(int maxnum)
{

	SockLib::CAutoLock al(m_ConnectedPeerListMutex);

	//in PriorityQueue small value is on top like 1,2,3...
	std::priority_queue<CBTPeer*, std::vector<CBTPeer*>, _DownloadCompare> PriorityQueue; //#include <queue>

	unsigned int now=GetTickCount();

	TPeerList::const_iterator it;

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
    {
        CBTPeer* peerLink = (*it);

		assert(peerLink!=NULL);

		if(!peerLink->IsGotBitSet()) continue;

		//so the peer will have more chance to get our upload.
		if( now - peerLink->GetBitFieldTime() > 60*1000	//̫ʱӱر
			&& peerLink->IsMeInterestPeer() 
			)
		{
			peerLink->CalculateDownloadPriority();
			PriorityQueue.push(peerLink); //here we have trouble
		}

    }

	if(PriorityQueue.size() <= m_pSession->GetLinkMax() / 4  ) //KEEPLINK=totallink/5
	{
		return 0;
	}

	int count=0;

	for(int i=0;!PriorityQueue.empty() && i<maxnum; i++)
	{
		PriorityQueue.top()->ClosePeer(CR_PICKCLOSE);//ѡԹر, ˵Էȼⲻ
		PriorityQueue.pop();
		count++;
	}

	return count;

}

//check for close we not interested peer
//رղȤҲӦŶӹرգиȴ

class _NoInterestCompare
{
public:
  bool operator() (  CBTPeer*& lclient,  CBTPeer*& rclient ) 
  {
    return (lclient->GetNotInterestPriority() > rclient->GetNotInterestPriority());
  }
};

int CPeerAdmin::CloseNotInterestPeer(int maxnum)
{

	SockLib::CAutoLock al(m_ConnectedPeerListMutex);
	//in PriorityQueue small value is on top like 1,2,3...
	std::priority_queue<CBTPeer*, std::vector<CBTPeer*>, _NoInterestCompare> PriorityQueue; //#include <queue>

	TPeerList::iterator it;


	//close the link that we are not interest and be choked
	for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end() ; it++)
	{		
		CBTPeer* peerLink=(*it);

		//we are not interest the peer, close it
		if( peerLink->IsGotBitSet() && !(peerLink->IsMeInterestPeer()) )
		{
			PriorityQueue.push(peerLink); //Ŷ
		}

	}
	
//	if(PriorityQueue.empty()) return 0;  //ûκŶ

	int count=0;

	for(int i=0;!PriorityQueue.empty() && i<maxnum; i++)
	{
		PriorityQueue.top()->ClosePeer(CR_NOTINTEREST);//ȤĹر
		PriorityQueue.pop();
		count++;
	}

	return count;

}

void CPeerAdmin::CheckClosedConnection()
{
	//move closed in connected list to m_UnusedPeerList;
	TPeerList::iterator it;

    SockLib::CAutoLock al(m_ConnectedPeerListMutex);

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); )
    {
		CBTPeer* peerLink=(*it);

        if ( peerLink->GetLinkState() == LS_CLOSE )
        {

			bool bencrypt	=peerLink->IsEncrypt();
            bool accepted	=peerLink->IsAccepted();
			TCloseReason cr	=peerLink->GetCloseReason();
			std::string pid	=peerLink->GetPeerId();
			unsigned int iip=peerLink->GetPeeriIP();

			if(peerLink->IsGotBitSet())
			{
				CBTPiece bitset=peerLink->GetPeerBitSet();
				m_pSession->CloseReport(iip,cr,accepted,pid,&bitset);

			}
			else
			{
				m_pSession->CloseReport(iip,cr,accepted,pid,NULL);
				//syslog("not got bitset closed\n");
			}

			delete peerLink;
			
			//it= m_ConnectedPeerList.erase( it );
			m_ConnectedPeerList.erase(it++); 

            continue;

        }

        it++;

    }

}

void CPeerAdmin::CheckClosedConnecting()
{

	TPeerList::iterator it;

    for ( it = m_ConnectingPeerList.begin();it != m_ConnectingPeerList.end(); )
    {
		CBTPeer* peerLink=(*it);

        if ( peerLink->GetLinkState() == LS_CLOSE )
        {
			TCloseReason reason=peerLink->GetCloseReason();
			unsigned int iip= peerLink->GetPeeriIP();
			std::string peerid=peerLink->GetPeerId();

			delete peerLink;

			if(reason==CR_LINKFAIL)
			{//only report link fail
				m_pSession->LinkReport(iip,false);

			}
			else if(reason!=CR_NOROOM)
			{
				//m_pSession->LinkReport(iip,true);
				//m_pSession->CloseReport(iip,reason,0,0,false,NULL,NULL);
				m_pSession->LinkOkButPeerClose(iip);	//ԷֱӹرգҲǶԷֹͣ
				//syslog("peer close link-1\n");
			}
			else
			{//report link ok and link close
				//assert(reason==CR_NOROOM);	//,бĹرԭвֻ
				//ԷرҲ޿ռر
				m_pSession->LinkOkButNoRoomClose(iip); //peeridûУΪǶû
				//syslog("peer close link-2\n");
			}

			m_ConnectingPeerList.erase(it++); //for vc8 test

            continue;

        }

        it++;
    }

}

void CPeerAdmin::CheckConnectedConnecting()
{
    // move new connection to m_ConnectedPeerList
	TPeerList::iterator it;
    for ( it = m_ConnectingPeerList.begin(); it != m_ConnectingPeerList.end(); )
    {
		CBTPeer* peerLink=(*it);

		assert(peerLink);

        if ( peerLink->GetLinkState() == LS_CONNOK )
        {
			unsigned int iip=peerLink->GetPeeriIP();

			//Ƿ񳬱ꣿ
			if(m_ConnectedPeerList.size() >= m_pSession->GetLinkMax())
			{
				//ˣȫصԹرһЧ
				if(0==CloseNotInterestPeer(1))
				{
					CloseInterestedPeer(1);
				}

				break;

			}

            m_ConnectedPeerListMutex.Lock();
            m_ConnectedPeerList.push_back(peerLink);
            m_ConnectedPeerListMutex.Unlock();

#ifdef _CHECK
			//if(m_ConnectedPeerList.size() > m_pSession->GetLinkMax())
			//{
			//	OutMsg(L"error!!",MSG_ERROR);
			//}
#endif
			peerLink->DownloadFinish(m_bDownloadFinish);

			peerLink->MoveToConnectedList(); //2007/10/07 ֹ緢ֱ¶ID
			//report to center
			m_pSession->LinkReport(iip,true);

			m_ConnectingPeerList.erase(it++); 

            continue;
        }

        it++;
    }

	//Щӵĵûλÿˣص
	for(it = m_ConnectingPeerList.begin(); it != m_ConnectingPeerList.end(); it++)
	{
		CBTPeer* peerLink=(*it);

        if ( peerLink->GetLinkState() == LS_CONNOK )
        {
			peerLink->ClosePeer(CR_NOROOM);
		}
	}

}

void CPeerAdmin::LaunchNewConnectionWhenFinished()
{

//㷨ѡٵʧܴĽڵӣҪһ
//ѡмһϵĽڵ㣬ʧܴѡǰļ


	int c=m_pSession->GetUploadLinkMax() - m_ConnectedPeerList.size();

	unsigned int iip;
	unsigned short iport;
	int encref;
	unsigned int timeout;

	//part 1:
	while( c-- > 0 && m_pSession->GetPeerInfoToLink(false, iip,iport,encref,timeout))
	{


		CBTPeer* peerLink = new CBTPeer(this);

/*/		

		switch(m_EncryptionMode)
		{
		case _ALWAYS_ENCRYPT:
			peerLink->SetEncrypt(true,true);
			break;
		case _ALWAYS_ORDINARY:
			break;
		case _PREFER_ENCRYPT:
			if(encref%2) 
			{//encryption have fail one time, try normal connect
				//dont set encrypt
			}	
			else
			{//not 
				peerLink->SetEncrypt(true,true); //if this conn fail ,set the EncryptRef mark
			}
			break;
		case _PREFER_ORDINARY:
		default:
			if(encref%2)
			{
				peerLink->SetEncrypt(true,true);
			}	
			break;
		}
/*/
		if(encref%2) 
		{//Ǽ
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
		else
		{//˫Ϊ
			peerLink->SetEncrypt(true,true);
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
//*/
		if(peerLink->CreateSock())
		{
			peerLink->SetDealer(GetDealer());
			peerLink->Connect( iip, iport, timeout );
			peerLink->DownloadFinish(true);
			
			m_ConnectingPeerList.push_back(peerLink);
		}	
		else
		{
			delete peerLink;
		}
		
	}

	//part2:
	int limit=0;
	while( c-- > 0 && m_pSession->GetPeerInfoToLink(true, iip,iport,encref,timeout))
	{
		if(++limit>2)
		{
			m_pSession->GiveUpLink(iip);
			return;
		}


		CBTPeer* peerLink = new CBTPeer(this);

		
/*/
		switch(m_EncryptionMode)
		{
		case _ALWAYS_ENCRYPT:
			peerLink->SetEncrypt(true,true);
			break;
		case _ALWAYS_ORDINARY:
			break;
		case _PREFER_ENCRYPT:
			if(encref%2) 
			{//encryption have fail one time, try normal connect
				//dont set encrypt
			}	
			else
			{//not 
				peerLink->SetEncrypt(true,true); //if this conn fail ,set the EncryptRef mark
			}
			break;
		case _PREFER_ORDINARY:
		default:
			if(encref%2)
			{
				peerLink->SetEncrypt(true,true);
			}	
			break;
		}
/*/
		if(encref%2) 
		{//Ǽ
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
		else
		{//˫Ϊ
			peerLink->SetEncrypt(true,true);
			if(encref<=0)
			{//δȷԷǷּ֧
				peerLink->ReportEncryptable(true);
			}
		}
//*/		
		if(peerLink->CreateSock())
		{
			peerLink->SetDealer(GetDealer());
			peerLink->Connect( iip, iport, timeout );
			peerLink->DownloadFinish(true);
			
			m_ConnectingPeerList.push_back(peerLink);
		}	
		else
		{
			delete peerLink;
		}
		
	}

}


void CPeerAdmin::SwitchMode()
{

	TPeerList::iterator it;

	SockLib::CAutoLock al(m_ConnectedPeerListMutex);

	for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
	{
		(*it)->DownloadFinish(m_bDownloadFinish); //tell peer cancel the data request
	}

	m_bSwitchMode=false; //we finish the switch, set the mark to false
	

}

void CPeerAdmin::SetEncryptionMode(_BT_ENCMODE mode)
{
	m_EncryptionMode=mode;
}


//#ifdef _CHECK
//void CPeerAdmin::OutMsg(wchar_t *msg, _MSGTYPE type)
//{
//	m_pSession->LogMsg(msg,type);
//}
//#endif

SockLib::CDealer* CPeerAdmin::GetDealer()
{
	return m_pSession->GetDealer();
}

CSpeedControlBase* CPeerAdmin::GetSpeedControl()
{
	return m_pSession->GetSpeedControl();
}

bool CPeerAdmin::GotHash(std::string hash, CBTPeer* client)
{
	std::string myhash=m_pSession->GetStorage()->GetTorrentFile()->GetInfoHash();
	if(hash!=myhash) {
		return false;
	}

	return true; //
}

//ڼתƹӣܾͷ棬ܷܽؼ
//AddAcceptPeer
bool CPeerAdmin::TransferPeer(CBTPeer *peerLink)
{

	//Ӧǰ
	unsigned iip=peerLink->GetPeeriIP();

	SockLib::CAutoLock al(m_ConnectedPeerListMutex);

	if(!m_bDownloadFinish)
	{
		if(m_ConnectedPeerList.size() >= m_pSession->GetLinkMax())
		{//peercenter֪жٸȤͲȤ
	
			//̹߳رӻ, ȡ
			//if(CloseNotInterestPeer(1)==0 && CloseInterestedPeer(1)==0)
			{
				//accept
				return false;
			}
		}
	}
	else
	{
		if(m_ConnectedPeerList.size() >= 3* m_pSession->GetUploadLinkMax())
		{

			//̹߳رӻ, ȡ
			//if(CloseLowPriorityUploadPeer(1)==0) 
			{
				//accept
				return false;
			}

		}
	}


	if(!m_pSession->TryAcceptPeerLink(iip)) return false;


	peerLink->SwitchAdmin(this); //лԱ

	//tell peer if we are finished
	peerLink->DownloadFinish(m_bDownloadFinish);

    m_ConnectedPeerList.push_back(peerLink);  //peerid is calculate by port=0

//#ifdef _CHECK
//			if(m_ConnectedPeerList.size() > m_pSession->GetLinkMax())
//			{
//				OutMsg(L"error------2!!",MSG_ERROR);
//			}
//#endif

    return true;

}

bool CPeerAdmin::GotEncryptHash(std::string hashxor, MSE::BigInt dhsecrect,CBTPeer* client)
{
	return true; //
}




//ƣرһЩϴȵ͵ӣֻϴģʽе
int CPeerAdmin::CloseLowPriorityUploadPeer(int maxnum)
{
	SockLib::CAutoLock al(m_ConnectedPeerListMutex);
	//in PriorityQueue small value is on top like 1,2,3...
	std::priority_queue<CBTPeer*, std::vector<CBTPeer*>, _UploadCompare> PriorityQueue; //#include <queue>

	unsigned int now=GetTickCount();

	//TPeerInfoMap::iterator it;
	TPeerList::iterator it;
    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++ )
    {
        CBTPeer* peerLink = (*it);

		if(peerLink->IsGotBitSet()) //ϴģʽУbitsetͿȼ
		{
			peerLink->CalculateUploadPriority();
			PriorityQueue.push(peerLink);
		}

    }


	int count=0;

	for(int i=0;!PriorityQueue.empty() && i<maxnum; i++)
	{
		PriorityQueue.top()->ClosePeer(CR_PICKCLOSE);//ѡԹر
		PriorityQueue.pop();
		count++;
	}

	return count;
}


void CPeerAdmin::CloseTimeoutConnection()
{

//Ӳʱ3
	TPeerList::iterator it;
    SockLib::CAutoLock al(m_ConnectedPeerListMutex);

	unsigned int now=GetTickCount();

    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++)
    {
		CBTPeer* peerLink=(*it);

        if ( peerLink->DataTimeoutCheck(now, 180*1000) )
        {
			peerLink->ClosePeer(CR_DATA_TIMEOUT);
        }

    }

	if(!m_pSession->AnyUnCheckedNode()) return;

	//δĽڵ㣬ʱΪ90
    for ( it = m_ConnectedPeerList.begin();it != m_ConnectedPeerList.end(); it++)
    {
		CBTPeer* peerLink=(*it);

        if ( peerLink->DataTimeoutCheck(now, 90*1000) )
        {
			peerLink->ClosePeer(CR_DATA_TIMEOUT);
        }

    }

}
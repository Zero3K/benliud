/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// TrackerCenter.cpp: implementation of the CTrackerCenter class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif
#include "../include/TrackerCenter.h"
#include <Dealer.h>
#include "../include/BTStorage.h"
#include <TorrentFile.h>
#include "../include/TCPTracker.h"
#include "../include/UDPTracker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define _TCP_TRACKER_CONNECTING_LIMIT (5)

CTrackerCenter::CTrackerCenter(CBTStorage* parent)
{
	m_pParent=parent;
	m_bStop=false;
	m_pTrackerStatus=NULL;
	m_ConnectingCount=0;
}

CTrackerCenter::~CTrackerCenter()
{
	if(m_pTrackerStatus!=NULL) 
	{
		delete[] m_pTrackerStatus;
	}
}

void CTrackerCenter::Start()
{

	m_bStop=false;

	if(m_pTrackerStatus!=NULL) 
	{
		delete m_pTrackerStatus;
		m_pTrackerStatus=NULL;
	}

	m_pDealer=new SockLib::CDealer;

	Run(false);
}

void CTrackerCenter::Entry()
{

	int annum=m_pParent->GetTorrentFile() ->GetAnnounceNumber();
	
	m_pTrackerStatus=new int[annum+6];
	memset(m_pTrackerStatus,0,sizeof(int)*(annum+6));

    for ( int i = 0; i < annum; i++ )
    {
		if(m_bStop) return; //some time stop in the middle we start tracker

        std::string trackerUrl = m_pParent->GetTorrentFile() ->GetAnnounce( i );

#ifdef WIN32

        if ( _strnicmp( "http", trackerUrl.c_str(), 4 ) == 0 )
#else

        if ( strncasecmp( "http", trackerUrl.c_str(), 4 ) == 0 )
#endif
        {

            CTCPTracker * tracker1 = new CTCPTracker( this ,i+1);
			tracker1->SetDealer(m_pDealer);
            tracker1->SetTracker( trackerUrl );
			tracker1->SetHash((char*)(m_pParent->GetTorrentFile()->GetInfoHash().data()));
			tracker1->SetId((char*)m_pParent->GetMyID(0));

			tracker1->Start();
            m_TrackerList.push_back( tracker1 );

        }
#ifdef WIN32
        else if ( _strnicmp( "udp", trackerUrl.c_str(), 3 ) == 0 )
#else

        else if ( strncasecmp( "udp", trackerUrl.c_str(), 3 ) == 0 )
#endif

        {
			//start 1 copys

            CUDPTracker * tracker1 = new CUDPTracker( this ,i+1);
			tracker1->SetDealer(m_pDealer);
            tracker1->SetTracker( trackerUrl );
			tracker1->SetHash((char*)(m_pParent->GetTorrentFile()->GetInfoHash().data()));
			tracker1->SetId((char*)m_pParent->GetMyID(0));

			tracker1->Start();
            m_TrackerList.push_back( tracker1 );

        }

    }


	//and here goes a constant tracker: udp://tracker.thepiratebay.org:80/announce
	/////////////////
	CUDPTracker * tpb = new CUDPTracker( this , annum);
	tpb->SetDealer(m_pDealer);
	//这个还有没有用值得怀疑了!
	tpb->SetTracker( "udp://tracker.thepiratebay.org:80/announce" );
	tpb->SetHash((char*)(m_pParent->GetTorrentFile()->GetInfoHash().data()));
	tpb->SetId((char*)m_pParent->GetMyID(0));
	tpb->Start();
	m_TrackerList.push_back(tpb);
	/////////////////

	//and here goes a single DHT tracker
	while(!m_bStop)
	{
		m_pDealer->Dispatch();
	}	

	delete[] m_pTrackerStatus;
	m_pTrackerStatus=NULL;
}

void CTrackerCenter::Stop()
{

	m_bStop=true;
	Wait();

	//delete all the object
	TTrackerList::iterator it;
	for(it=m_TrackerList.begin();it!=m_TrackerList.end();it++)
	{
		(*it)->Stop();
		delete (*it);
	}

	m_TrackerList.clear();

	delete m_pDealer;
	m_pDealer=NULL;

}


SockLib::CDealer* CTrackerCenter::GetDealer()
{
	return m_pDealer;
}

bool CTrackerCenter::AddPeer(unsigned int iip, unsigned short iport)
{
	return m_pParent->AddNewPeer(iip,iport);
	
}

void CTrackerCenter::NoticePeerAndSeed(int total, int seed)
{

}

CBTStorage* CTrackerCenter::GetStorage()
{
	return m_pParent;
}


void CTrackerCenter::SetEvent(TTrackerEvent event)
{
	//set to all tracker
	m_CurEvent=event;
}

//trackers call this to report its status
//status < 0 wrong
//status ==0 not started
//status > 0 number of peers got
void CTrackerCenter::ReportTrackerStatus(int trackerseq, int status)
{
	m_pTrackerStatus[trackerseq]=status;

}

//write the status code to buffer
void CTrackerCenter::GetTrackerStatus(int buflen, int *status)
{
	if(m_pTrackerStatus==NULL) 
	{
		for(int i=0;i<buflen;i++)
		{
			status[i]=0;
		}
		return ;
	}
	
	for(int i=0;i<buflen;i++)
	{
		status[i]=m_pTrackerStatus[i];
	}
}

bool CTrackerCenter::GetServerIP(std::string server, std::string &ip)
{
	return m_DNSBuffer.GetServerIP(server,ip);
}

//为限制同时连接tracker的数量太多而影响主体
//增加两个函数, TCPTracker连接前调用这个，
//连接成功或失败时调用ReleaseConnectingHandle()
//如果没取到权限，则放空到下次连接
bool CTrackerCenter::GetConnectingHandle()
{
	SockLib::CAutoLock al(m_ConnectingCountMutex);
	if(m_ConnectingCount >=_TCP_TRACKER_CONNECTING_LIMIT) return false;
	
	m_ConnectingCount++;
	return true;
}

void CTrackerCenter::ReleaseConnectingHandle()
{
	SockLib::CAutoLock al(m_ConnectingCountMutex);
	m_ConnectingCount--;
	assert(m_ConnectingCount>=0);
}

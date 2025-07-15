/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

// UDPTracker.cpp: implementation of the CUDPTracker class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/UDPTracker.h"
#include "../include/TrackerCenter.h"
#include <Dealer.h>
#include "../include/BTStorage.h"
#include <TorrentFile.h>
#include <Tools.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*
#define TS_NOTRUN		(0)
#define TS_CONNECTING (-1)
#define TS_CONNECTTED (-2)
#define TS_HEADERERR  (-3)
#define TS_CONTENTERR	(-4)
#define TS_GOTERRMSG  (-5)
*/
CUDPTracker::CUDPTracker(CTrackerCenter* parent, int trackerseq)
{
	m_pParent=parent;

	m_TimerId=0;
	maskRead(false);
	maskWrite(false);

	m_CurEvent=TE_START;
	m_nTrackerSeq=trackerseq;

}

CUDPTracker::~CUDPTracker()
{

}

bool CUDPTracker::Start()
{
    m_nTransID = rand() * GetTickCount();

	m_State=TS_INIT;
	m_nFailCount=0;
	m_CurEvent=TE_START;
	//gethostbyname
	std::string host;
	std::string path;
	unsigned short port;
	
	if ( !Tools::parseUrl( m_Tracker.c_str(), host, &port, path ) )
	{
		return false;
	}

	std::string sip;
	if(!m_pParent->GetServerIP(host,sip))
	{
		return false;
	}

	if(!CreateSock()) return false;

	//SetDealer(m_pParent->GetDealer()); 

	m_Server.iip=inet_addr(sip.c_str());
	m_Server.iport=htons( port );

	SendConnectPacket();

	return true;
}

void CUDPTracker::Stop()
{
	//if(m_CurEvent==TE_START) return;
	//SendStopPacketWhenQuit();
	Close();
}

void CUDPTracker::SendConnectPacket()
{

	typedef struct
	{
		llong connection_id;
		unsigned int action_id;
		unsigned int transaction_id;
	}TConnectPacket;

    TConnectPacket pkg;
#ifdef WIN32
    pkg.connection_id = 0x41727101980L; //connect id 
#else
    pkg.connection_id = 0x41727101980LL; //connect id	
#endif
    pkg.connection_id = htonll( pkg.connection_id );
    pkg.action_id = 0;
    pkg.transaction_id = htonl( m_nTransID );

	Send(( const char* ) & pkg, sizeof(pkg), m_Server.iip, m_Server.iport);

	maskRead(true);
	maskWrite(false);

	m_State=TS_CONNECTING;

	m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTING);

	m_TimerId=AddTimer(60*1000,true);
}

llong CUDPTracker::htonll( llong number )
{
    if ( htons( 1 ) == 1 )
    {
        return number;
    }

    return ( htonl( ( number >> 32 ) & 0xFFFFFFFF ) |
             ( ( llong ) ( htonl( number & 0xFFFFFFFF ) ) << 32 ) );
}


void CUDPTracker::SetTracker(std::string tracker)
{
	m_Tracker=tracker;
}

void CUDPTracker::OnTimer(unsigned int id)
{

	if(m_TimerId==id)
	{
		if(m_State==TS_INIT)
		{
//normal connect
			SendConnectPacket(); //it will set the timer
		}
		else if(m_State==TS_CONNECTING)
		{
//connecting timeout

			m_nFailCount++;
			m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTFAIL);

			RemoveTimer(m_TimerId);
			//next time update
			m_State=TS_INIT;

			m_TimerId=AddTimer(GetInterval());
		}
		else if(m_State==TS_OK)
		{//now send request again
//normal request.
			SendRequestPacket();  //it will set the next update timer
		}
		else if(m_State==TS_ERROR)
		{//error retry
//	last request error, request again.
			SendRequestPacket();
		}
		else if(m_State==TS_REQUESTING)
		{
//requesting timeout
			m_nFailCount++;
			m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_REQTIMEOUT);

			m_State=TS_ERROR;

			//set the retry time
			m_TimerId=AddTimer(GetInterval());
		}
	}
}

int CUDPTracker::GetInterval()
{
	int interval=1;
	for(unsigned int i=0;i<m_nFailCount;i++)
	{
		interval*=2;
	}
	return interval*30*1000+(rand()%10)*1000;
}

void CUDPTracker::OnRead()
{
    char buf[ 8*1024 ];

	SockLib::TInetAddr4 peer;

	int ret = Recv( buf, 8*1024, peer.iip, peer.iport);

	if( ret <16 ) return;

	//unsigned int actionID = *( ( unsigned int* ) buf );
	unsigned int actionID;
	memcpy(&actionID, buf, sizeof(unsigned int));
	actionID = ntohl( actionID );
	
	//unsigned int transID = *( ( unsigned int* ) ( buf + 4 ) );
	unsigned int transID;
	memcpy(&transID, buf+4, sizeof(unsigned int));
	transID = ntohl( transID );
	
	if ( transID == m_nTransID )
	{
		ParseResponse( actionID, buf + 8, ret - 8 );
	}

    return ;
}

void CUDPTracker::OnWrite()
{
	maskWrite(false);
}

void CUDPTracker::OnClose()
{
	CSockProxyUDPClientSock::OnClose();
}

void CUDPTracker::ParseResponse( unsigned int actionID, char* data, size_t len )
{

    if ( actionID == 3 )
    {//Ϣ
		if(m_State==TS_CONNECTING) //connect packet return error message
		{
			m_State=TS_INIT;
		}
		else
		{
			m_State=TS_ERROR;
		}

		m_nFailCount++;

		m_TimerId=AddTimer(GetInterval());

		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_GOTERRMSG);

        return ;
    }
    else if ( actionID == 0 )
    {//ȷid
		m_nFailCount=0;
        //m_connectionID = *( ( llong* ) data );
		memcpy(&m_connectionID, data, sizeof(llong));
        m_connectionID = ntohll( m_connectionID );
        m_State = TS_OK;

//		OutMsg("connect ok,send our request",MSG_INFO);

		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTTED);
		SendRequestPacket();
        return ;
    }

    else if ( actionID == 1 )
    { 
		
/*
output		Offset		Size	Name	Value  
0			32-bit		integer action	1  
4			32-bit		integer transaction_id  
8			32-bit		integer interval  
12			32-bit		integer leechers  
16			32-bit		integer seeders  
20 + 6 * n	32-bit		integer IP address  
24 + 6 * n	16-bit		integer TCP port  
20 + 6 * N  
*/

		m_nFailCount=0;
		m_State = TS_OK;

        //unsigned int interval = *( ( unsigned int* ) data );
		unsigned int interval;
		memcpy(&interval, data, sizeof(unsigned int));
        interval = htonl( interval );

        //unsigned int leechers = *( ( unsigned int* ) ( data + 4 ) );
		unsigned int leechers;
		memcpy(&leechers, data+4, sizeof(unsigned int));

        leechers = htonl( leechers );

        //unsigned int seeders = *( ( unsigned int* ) ( data + 8 ) );
		unsigned int seeders;
		memcpy(&seeders, data+8, sizeof(unsigned int));

        seeders = htonl( seeders );


        char* pbuf = NULL;

        for ( unsigned int i = 0; i < ( len - 12 ) / 6; ++i )
        {
			std::string ipport;
			ipport.append(data+12+i*6,6);

			//unsigned int iip=(*(unsigned int*)(ipport.data()));
			unsigned int iip;
			memcpy(&iip, ipport.data(), sizeof(unsigned int));
			//unsigned short iport=(*(unsigned short*)(ipport.data()+4));
			unsigned short iport;
			memcpy(&iport, ipport.data()+4, sizeof(unsigned short));

			m_pParent->AddPeer(iip,iport);

        }
        

        if ( interval <= 150 )
        {
            RemoveTimer(m_TimerId);
			m_TimerId=AddTimer(interval*1000+(rand()%20)*1000);
        }
        else
        {
            RemoveTimer(m_TimerId);
			m_TimerId=AddTimer(150*1000+(rand()%20)*1000);          
        }

		m_CurEvent=TE_NONE;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,(len-12)/6);

    }
	else if( actionID == 2 ) //scrape 
	{

/*
scrape output 
Offset		Size	Name	Value  
0			32-bit	integer action 2  
4			32-bit	integer transaction_id  
8 + 12 * n	32-bit	integer seeders  
12 + 12 * n 32-bit	integer completed  
16 + 12 * n 32-bit	integer leechers  
8 + 12 * N  

If the tracker encounters an error, it might

*/
		m_nFailCount=0;
		m_State = TS_OK;

		m_TimerId=AddTimer(60*1000+(rand()%20)*1000); 

        m_CurEvent=TE_NONE;

	}
}

llong CUDPTracker::ntohll( llong number )
{
    if ( htons( 1 ) == 1 )
    {
        return number;
    }

    return ( htonl( ( number >> 32 ) & 0xFFFFFFFF ) |
             ( ( llong ) ( htonl( number & 0xFFFFFFFF ) ) << 32 ) );
}

void CUDPTracker::SendRequestPacket()
{
	
    m_nTransID = rand() * GetTickCount();

	typedef struct
	{
		llong connection_id;
		unsigned int action_id;
		unsigned int transaction_id;
		char info_hash[ 20 ];
		char peer_id[ 20 ];
		llong downloaded;
		llong left;
		llong uploaded;
		unsigned int event;
		unsigned int ip;
		unsigned int key;
		int num_want;
		unsigned short port;
		unsigned short extensions;
	}TRequestPacket;

    TRequestPacket pkg;
    pkg.connection_id = htonll( m_connectionID ); //õID
    pkg.action_id = htonl( 1 ); //̶Ϊ1
    pkg.transaction_id = htonl( m_nTransID );

    memcpy( pkg.info_hash, m_Hash, 20 );

    memcpy( pkg.peer_id, m_Id, 20 );

    pkg.downloaded = htonl( m_pParent->GetStorage()->GetSumOfDownload() );
    pkg.left = htonl( m_pParent->GetStorage() ->GetTorrentFile() ->GetTotalSize() - m_pParent->GetStorage() ->GetSumOfDownload() );
    pkg.uploaded = htonl( m_pParent->GetStorage() ->GetSumOfUpload() );


	if(m_CurEvent==TE_START)
	{
		pkg.event=htonl(2);
	}
	else if(m_CurEvent==TE_COMPLETE)
	{
		pkg.event=htonl(1);
	}
	else if(m_CurEvent==TE_STOP)
	{
		pkg.event=htonl(3);
	}
	else
	{
		pkg.event=htonl(0); //none
	}

    pkg.ip = htonl( 0 );//our ip default to 0

    pkg.key = htonl( m_nTransID * rand() );

    pkg.num_want = htonl( 200 );
    pkg.port = htons( (unsigned short)(m_pParent->GetStorage()->GetListenPort()) );
    pkg.extensions = 0;

	Send(( const char* ) & pkg, sizeof( pkg ), m_Server.iip, m_Server.iport);

	m_State=TS_REQUESTING;

	if(m_TimerId!=0)
	{
		RemoveTimer(m_TimerId);
	}

	m_TimerId=AddTimer(60*1000);
}

void CUDPTracker::SetId(char *id)
{
	memcpy(m_Id,id,20);
}

void CUDPTracker::SetHash(char *hash)
{
	memcpy(m_Hash,hash,20);
}
//

void CUDPTracker::SendStopPacketWhenQuit()
{
	
    m_nTransID = rand() * GetTickCount();

	typedef struct
	{
		llong connection_id;
		unsigned int action_id;
		unsigned int transaction_id;
		char info_hash[ 20 ];
		char peer_id[ 20 ];
		llong downloaded;
		llong left;
		llong uploaded;
		unsigned int event;
		unsigned int ip;
		unsigned int key;
		int num_want;
		unsigned short port;
		unsigned short extensions;
	}TRequestPacket;


    TRequestPacket pkg;
    pkg.connection_id = htonll( m_connectionID ); //õID
    pkg.action_id = htonl( 1 ); //̶Ϊ1
    pkg.transaction_id = htonl( m_nTransID );

    memcpy( pkg.info_hash, m_Hash, 20 );

    memcpy( pkg.peer_id, m_Id, 20 );

    pkg.downloaded = htonl( 0 );
    pkg.left = htonl( 0 );
    pkg.uploaded = htonl( 0 );

	pkg.event=htonl(3); //3 is stop event

    pkg.ip = htonl( 0 );//our ip default to 0

    pkg.key = htonl( m_nTransID * rand() );

    pkg.num_want = htonl( 0 );
    pkg.port = htons( 0 );
    pkg.extensions = 0;

	Send(( const char* ) & pkg, sizeof( pkg ), m_Server.iip, m_Server.iport);

}

void CUDPTracker::SetEvent(TTrackerEvent event)
{
	m_CurEvent=event;
}

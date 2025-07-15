/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#include "stdafx.h"

// TCPTracker.cpp: implementation of the CTCPTracker class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/TCPTracker.h"
#include "../include/HttpUrl.h"
#include "../include/TrackerCenter.h"
#include <Dealer.h>
#include "../include/BTStorage.h"
#include <Tools.h>
#include <TorrentFile.h>

#include <assert.h>
/*


#define TS_NOTRUN		(0)
#define TS_CONNECTING   (-1)
#define TS_CONNECTFAIL (-2)
#define TS_CONNECTTED (-3)
#define TS_HEADERERR  (-4)
#define TS_CONTENTERR	(-5)
#define TS_REQTIMEOUT (-6)
#define TS_GOTERRMSG  (-7)
	
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPTracker::CTCPTracker(CTrackerCenter* parent, int trackerseq)
{
	m_pParent=parent;

	m_Status=_TRACKER_WAITING;
	m_TimerId=0;
	maskRead(false);
	maskWrite(false);

	m_bRedirect=false;
	m_CurEvent=TE_START;
	m_nFailCount=0;
	m_nTrackerSeq=trackerseq;
}

CTCPTracker::~CTCPTracker()
{

}

void CTCPTracker::OnTimer(unsigned int id)
{
	CSockProxyTCPClientSock::OnTimer(id);

	if(m_TimerId==id)
	{
//		if(m_Status==_TRACKER_CONNECTING)
//		{//connect timeout
			//set some error message
//			m_nFailCount++;
//			m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTFAIL);
//			SetNextUpdate(GetInterval()+rand()%20);		
//		}
//		else 
		if(m_Status==_TRACKER_SENDED)
		{//reveive timeout
			//set some error message
			//OutMsg(L"response timeout",MSG_ERROR);
			m_nFailCount++;
			m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_REQTIMEOUT);
			SetNextUpdate(GetInterval()+rand()%20);	
		}
		else if(m_Status==_TRACKER_WAITING)
		{//the time to update
			Update();
		}
	}
}

bool CTCPTracker::Start()
{
	m_nFailCount=0;
	
	CHttpUrl parser((char*)(m_Tracker.c_str()));
	if(parser.IsOk())
	{
		m_nPort=parser.GetPort();
		m_EscPathFile=parser.GetEscPathFile();
		m_sServer=parser.GetServer();
		Update();
		return true;
	}
	else
	{
		//OutMsg("tracker url fail",MSG_ERROR);
		//OutMsg((char*)m_Tracker.c_str(),MSG_ERROR);
		return false;
	}
	
}

void CTCPTracker::OnRead()
{
	CSockProxyTCPClientSock::OnRead();
	if(!CanRead()) return;

//    int readCount = 0;

    char buf[ 16 * 1024 ];  //16k buffer

	while(1)
	{

#ifdef WIN32		
        int ret = recv(m_hSocket, buf, 16*1024, 0 );
#else
		int ret = read(m_hSocket, buf, 16*1024);
#endif
        if ( ret == 0 )
        {
            OnClose();
			return;
        }

#ifdef WIN32
        else if ( ret == SOCKET_ERROR )
        {
#else
        else if ( ret == -1 )
        {
#endif

#ifdef WIN32

            if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

            if ( errno == EAGAIN )
#endif
            {
                break;  //wait for read more data
            }

#ifndef WIN32			
            if ( errno == EINTR )
            {
                continue;
           	}
#endif
			else
			{
				m_recvBuffer.resize(0);
				m_bRedirect=false;
				m_redirectUrl.resize(0);
				OnClose(); //error close
				return;
			}

        }

        m_recvBuffer.append( ( const char* ) buf, ret );
  
	}

}

void CTCPTracker::OnWrite()
{
	CSockProxyTCPClientSock::OnWrite();
	if(!CanWrite()) return;

	if(m_Status==_TRACKER_CONNECTING)
	{


	}
}

void CTCPTracker::OnClose()
{
	CSockProxyTCPClientSock::OnClose();

	m_bRedirect=false;
	m_redirectUrl.resize(0);

	if(m_recvBuffer.empty())
	{
		m_nFailCount++;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_HEADERERR);
		SetNextUpdate(GetInterval());
		
		return;
	}

	//try to find the header end pos

	size_t headpos = m_recvBuffer.find( "\r\n\r\n", 0 );
	if(headpos==std::string::npos)
	{
		//syslog("can't find header\n");

		//OutMsg(L"no header return from tracker.\n",MSG_ERROR);

		m_recvBuffer.resize(0);
		//treat as fail.
		m_nFailCount++;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_HEADERERR);
		SetNextUpdate(GetInterval());
	}

	
	std::string header;
	header=m_recvBuffer.substr(0,headpos+2); //headpos+2 is to include a '\r\n'


	bool bred;
	std::string redstr;
	
	if(ParseHeader(header,bred,redstr))
	{

		if(bred)
		{
			//syslog("redirection happen.\n");

			m_nFailCount=0;//good response
			CHttpUrl parser((char*)(redstr.c_str()));
			if(parser.IsOk())
			{

				m_redirectUrl=redstr;
				m_bRedirect=true;

				//SetDealer(NULL); //ÕâÀïÓÐŽí£¡£¡2008/01/23
				Close(); //close the old socket

				Update();
				return ;
			}
			else
			{
				//something wrong with redirection
				//syslog("redirection url wrong.\n");
				//set errro msg
						//treat as fail.
				m_nFailCount++;
				m_pParent->ReportTrackerStatus(m_nTrackerSeq,-1);
				SetNextUpdate(GetInterval());

			}			
		}
		else
		{
			std::string content;
			content=m_recvBuffer.substr(headpos+4);

			ParseContent(content); //set the failcount if need

			//m_bRedirect=false;
			//m_redirectUrl.resize(0);
			m_recvBuffer.resize(0);
		}
		

	}
	else
	{//parser header fail
		//syslog("header fail\n");
		m_recvBuffer.resize(0);
		//set some error mesg
		m_nFailCount++;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_HEADERERR);
		SetNextUpdate(GetInterval());
	}

}

void CTCPTracker::OnConnectFail()
{
	CSockProxyTCPClientSock::OnConnectFail();


	//Á¬œÓÊ§°ÜÊÍ·ÅÁ¬œÓÈšÏÞ
	m_pParent->ReleaseConnectingHandle(); 

	m_nFailCount++;
	m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTFAIL);
	SetNextUpdate(GetInterval());	
}

void CTCPTracker::OnConnectOk()
{
	CSockProxyTCPClientSock::OnConnectOk();
	if(!CanWrite()) return;
	
	//Á¬œÓ³É¹ŠÊÍ·ÅÁ¬œÓÈšÏÞ

	m_pParent->ReleaseConnectingHandle(); 

	//do the send.
	m_Status=_TRACKER_CONNECTED;
	
	maskRead(true);
	maskWrite(false);
	
	//OutMsg(L"tcp tracker connected ok",MSG_INFO);
	m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTTED);
	
	//send out our request and wait for response
	if(!m_bRedirect)
	{
		std::string request=BuildTrackerUrl(EventStr(GetCurrentEvent()));
		SendRequest(m_sServer,request);
	}
	else
	{
		CHttpUrl parser((char*)(m_redirectUrl.c_str()));
		
		SendRequest(parser.GetServer(),parser.GetEscPathFile());
	}
	
	m_Status=_TRACKER_SENDED;
	
	if(m_TimerId!=0)
	{
		RemoveTimer(m_TimerId);
		m_TimerId=0;
	}
	
	m_TimerId=AddTimer(60000,true); //wait 60s 
	
}

void CTCPTracker::SetTracker(std::string tracker)
{
	m_Tracker=tracker;

}

void CTCPTracker::Update()
{
	unsigned short port;
	std::string dest;
	
	//È¡µÃÁ¬œÓÈšÏÞ
	if(!m_pParent->GetConnectingHandle())
	{
		SetNextUpdate(1+rand()%5); //3s ºóÔÙ³£Ê¶Á¬œÓ
		return;
	}

	if(!m_bRedirect)
	{
		port = ( unsigned short ) m_nPort ;
		dest = m_sServer;
	}
	else
	{
		CHttpUrl parser((char*)(m_redirectUrl.c_str()));
		port =  ( unsigned short ) parser.GetPort();
		dest = parser.GetServer();	
	}


	if(!CreateSock()) return;

	//SetDealer(m_pParent->GetDealer());

	m_recvBuffer.resize(0);
	
	CSockProxyTCPClientSock::Connect(dest,port,12*1000);

	m_Status=_TRACKER_CONNECTING;

//tracker report event define
/*
#define TR_NOTRUN		(0)
#define TR_CONNECTING   (-1)
#define TR_CONNECTFAIL (-2)
#define TR_CONNECTTED (-3)
#define TR_HEADERERR  (-4)
#define TR_CONTENTERR	(-5)
#define TR_REQTIMEOUT (-6)
#define TR_GOTERRMSG  (-7)
	*/
	m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONNECTING);

	return ;
}

void CTCPTracker::SendRequest(std::string server,std::string request)
{

	SendHttpGetRequest(server.c_str(),request.c_str(),"");

}

void CTCPTracker::ParseContent(std::string response)
{
	//the link closed ,parse the buffer and clean it
    bool bGotPeerInfo = false;
    int nValidPeer = 0;

    BencodeLib::CTorrentFile tf;

    if ( 0 != tf.ReadBuf( ( char* ) ( response.c_str() ), response.size() ) )
    { //wrong response
		m_nFailCount+=2;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_CONTENTERR); //the content illegal
        SetNextUpdate(GetInterval());
        return ;
    }


	BencodeLib::CBenNode* fail = tf.GetRootNode().FindKeyValue( "failure reason" );

    if ( fail != NULL )
    {
#ifdef _CHECK
        //std::string sfail;
        //fail->GetStringValue( sfail );

		//show the message
		//OutMsg((char*)(sfail.c_str()), MSG_ERROR);
#endif
		m_nFailCount+=2;
		m_pParent->ReportTrackerStatus(m_nTrackerSeq,TR_GOTERRMSG);  //got failure message
        SetNextUpdate(GetInterval());
        return ;
    }

	//got good response ,set it
	m_CurEvent=TE_NONE;
	m_nFailCount=0;

    BencodeLib::CBenNode* interval = tf.GetRootNode().FindKeyValue( "interval" );

    if ( interval != NULL )
    {
        m_Interval = int(interval->GetIntValue());
    }
	else
	{
		m_Interval=120;
	}


    BencodeLib::CBenNode* done = tf.GetRootNode().FindKeyValue( "done peers" );

    BencodeLib::CBenNode* total = tf.GetRootNode().FindKeyValue( "num peers" );

    if ( done != NULL && total != NULL )
    {

		m_pParent->NoticePeerAndSeed( int(total->GetIntValue()), int(done->GetIntValue()) );

    }

    BencodeLib::CBenNode* complete = tf.GetRootNode().FindKeyValue( "complete" );

    BencodeLib::CBenNode* incomplete = tf.GetRootNode().FindKeyValue( "incomplete" );

    if ( complete != NULL && incomplete != NULL )
    {
		m_pParent->NoticePeerAndSeed( int(complete->GetIntValue()) + int(incomplete->GetIntValue()),
                                      int(complete->GetIntValue()) );

    }


    unsigned int peerCount = 0;

    BencodeLib::CBenNode* peersObj = tf.GetRootNode().FindKeyValue( "peers" );


    if ( peersObj != NULL	&& peersObj->GetType() == BencodeLib::beList )
    {
        peerCount = peersObj->GetNumberOfList();
		m_pParent->NoticePeerAndSeed( peerCount, 0 );

        bGotPeerInfo = true;

        for ( unsigned int i = 0; i < peerCount; i++ )
        {
            BencodeLib::CBenNode* peerdict = peersObj->GetListMember( i );

            BencodeLib::CBenNode* ipnode = peerdict->FindKeyValue( "ip" );

            std::string ipstr;
            ipnode->GetStringValue( ipstr );

			unsigned int iip= inet_addr(ipstr.c_str());

			if(iip==INADDR_NONE) {
				//OutMsg(L"tracker return invalid ip",MSG_ERROR);
				continue;
			}

            BencodeLib::CBenNode* portnode = peerdict->FindKeyValue( "port" );
            int port = int(portnode->GetIntValue());
            //notice new peers

			if ( m_pParent->AddPeer( iip, htons(port) ) )
			{
				nValidPeer++;
			}

        }

		m_pParent->ReportTrackerStatus(m_nTrackerSeq,peerCount);
    }

    if ( peersObj != NULL	&& peersObj->GetType() == BencodeLib::beString )
    {
        //compact data

        std::string peerStr;
        peersObj->GetStringValue( peerStr );

        if ( peerStr.size() % 6 == 0 )
        {
			m_pParent->NoticePeerAndSeed( peerStr.size() / 6, 0 );


            bGotPeerInfo = true;

            for ( unsigned int i = 0; i < peerStr.size() / 6; ++i )
            {
				std::string ipport=peerStr.substr(i*6,6);
				//unsigned int iip=(*(unsigned int*)(ipport.data()));
				unsigned int iip;
				memcpy(&iip, ipport.data(), sizeof(unsigned int));
				//unsigned short iport=(*(unsigned short*)(ipport.data()+4));
				unsigned short iport;
				memcpy(&iport, ipport.data()+4, sizeof(unsigned short));

				if(m_pParent->AddPeer(iip,iport))
				{
					nValidPeer++;
				}

            }

			m_pParent->ReportTrackerStatus(m_nTrackerSeq,peerStr.size()/6);
        }

    }

    //set next update
    if ( m_Interval <= 120 )
    {
        if ( bGotPeerInfo && nValidPeer == 0 )
        {
			SetNextUpdate(3*60+rand()%60);
        }
        else
        {
			SetNextUpdate(m_Interval + rand()%30);
        }
    }
    else
    {
        if ( bGotPeerInfo && nValidPeer == 0 )
        {
			SetNextUpdate(3*60+rand()%60);
        }
        else
        {
			SetNextUpdate(60+rand()%20);
        }
    }

}

//return false means fail to parse header or wrong header
bool CTCPTracker::ParseHeader(std::string header, bool& red,std::string& redstr)
{
	int contentlen;
	std::string redirection;

	int retcode=0;

    std::string prestr;

    size_t last = m_EscPathFile.find_last_of( '/' );

    if ( last != std::string::npos )
    {
        prestr = m_EscPathFile.substr( 0, last );
    }


	size_t pos_1, pos_2;
	pos_1=pos_2=0;

	//syslog("the header:\n");
	while((pos_2=header.find('\n',pos_1))!=std::string::npos)
	{
		//found the pos2
		std::string line=header.substr(pos_1,pos_2-pos_1);
		pos_1=pos_2+1; //reset the pos_1

		Tools::TrimStringRight(line,"\r");
		//syslog(line);
		//syslog("\n");


#if defined(WIN32)||defined(__WXMSW__)

        //if (  line.length() >= 12 && _strnicmp( line.c_str(), "HTTP/1.", 7 ) == 0 )
		if (line.length() >=12 && _strnicmp(line.c_str(), "HTTP/1.",7)==0)	//_strnicmp for vc8.0
#else

        if ( line.length() >= 12 && strncasecmp( line.c_str(), "HTTP/1.", 7 ) == 0 )
#endif
		{
			std::string scode=line.substr(9,3);
			Tools::TrimAllSpace(scode);
			retcode=atoi(scode.c_str());
		}
#ifdef WIN32
        //else if ( line.length() >= 16 && strnicmp( line.c_str(), "CONTENT-LENGTH:", 15 ) == 0 )
		else if ( line.length() >= 16 && _strnicmp( line.c_str(), "CONTENT-LENGTH:", 15 ) == 0 ) //_strnicmp for vc8.0
#else

        else if ( line.length() >= 16 && strncasecmp( line.c_str(), "CONTENT-LENGTH:", 15 ) == 0 )
#endif

        {

			std::string slen=line.substr(15);
			Tools::TrimAllSpace(slen);
            contentlen = atoi( slen.c_str() );

        }


#ifdef WIN32
//        else if ( line.length() >= 10 && _strnicmp( line.c_str(), "LOCATION:", 9 ) == 0 )
		else if ( line.length() >= 10 && _strnicmp( line.c_str(), "LOCATION:", 9 ) == 0 )
#else

        else if ( line.length() >= 10 && strncasecmp( line.c_str(), "LOCATION:", 9 ) == 0 )
#endif
        {

			redirection = line.substr(9);
			Tools::TrimAllSpace(redirection);

            if ( redirection.empty() )
            {
                return false; //no retry
            }

            if ( redirection[ 0 ] == '/' )  //not a full url
            {
                //some give /dir/to/file location
                redirection = std::string( "http://" ) + m_sServer + redirection;
            }
            else
            {
                //try to find "://" in string , if have , it's a full path

                size_t pos = redirection.find( "://", 0 );

                if ( pos == std::string::npos )
                {
                    //not a full url,append something to head
                    redirection = std::string( "http://" ) + m_sServer + prestr +
                                    std::string( "/" ) + redirection;
                }

            }

        }

	}


	if(retcode/100==3 && !redirection.empty())
	{
		red=true;
		redstr=redirection;
	//	OutMsg((char*)redirection.c_str(),MSG_INFO);
		return true;
	}
	else if(retcode!=200)
	{
		red=false;
		//wchar_t cod[128];
		//swprintf(cod,L"retcode=%d, tracker fail.",retcode);
		//OutMsg(cod,MSG_ERROR);
		return false;
	}

	red=false;
	return true;
}

void CTCPTracker::SetNextUpdate(int seconds)
{

	Close(); //close the old socket

	m_Status=_TRACKER_WAITING; //get into waiting

	if(m_TimerId!=0)	RemoveTimer(m_TimerId);

	m_TimerId=AddTimer(seconds*1000, true); //wait 60s retry

}

#define MAXCHARINLINE (256)

int CTCPTracker::GetBufLine( const char *buf, int start, int maxpos, char *line )
{

    for ( int i = 0 ; i <= maxpos - 2 - start;i++ )
    {
        if ( buf[ i + start ] == '\r' && buf[ i + start + 1 ] == '\n' )
        {
            memcpy( line, buf + start, i > MAXCHARINLINE - 3 ? MAXCHARINLINE - 2 : i );
            line[ i > MAXCHARINLINE - 2 ? MAXCHARINLINE - 1 : i ] = 0;
            return i + 2;
        }
    }

    return -1;

}
#undef MAXCHARINLINE


std::string CTCPTracker::BuildTrackerUrl(std::string eventstr)
{

    char result[ 1024 ];

#ifdef WIN32

    sprintf( result, "%s?info_hash=%s&peer_id=%s&port=%d&compact=1&downloaded=%I64d&uploaded=%I64d&left=%I64d&numwant=200&no_peer_id=1&port_type=wan",
             m_EscPathFile.c_str(),
             Tools::EscapeHash( ( unsigned char* ) ( m_Hash ), 20 ).c_str(),
             Tools::EscapeHash( ( unsigned char* ) ( m_Id ), 20 ).c_str(),
             m_pParent->GetStorage() ->GetListenPort(),
             m_pParent->GetStorage() ->GetFinishedBytes(),
             m_pParent->GetStorage() ->GetSumOfUpload(),
             m_pParent->GetStorage() ->GetUnFinishedBytes() );

#else

    sprintf( result, "%s?info_hash=%s&peer_id=%s&port=%d&compact=1&downloaded=%llu&uploaded=%llu&left=%llu&numwant=200&no_peer_id=1&port_type=wan",
             m_EscPathFile.c_str(),
             Tools::EscapeHash( ( unsigned char* ) ( m_Hash ), 20 ).c_str(),
             Tools::EscapeHash( ( unsigned char* ) ( m_Id ), 20 ).c_str(),
             m_pParent->GetStorage()->GetListenPort(),
             m_pParent->GetStorage() ->GetFinishedBytes(),
             m_pParent->GetStorage() ->GetSumOfUpload(),
             m_pParent->GetStorage() ->GetUnFinishedBytes() );

#endif

/*·¢ËÍIPÒâÒåÒ²²»Žó£¬»¹ÈÝÒ×³öŽížãŽíIPµØÖ·
    std::string myip ;

    if ( !m_pParent->GetStorage() ->GetExternIP(myip) )
        return std::string( result ) + eventstr;
    else
        return std::string( result ) + eventstr + std::string( "&ip=" ) + myip;
*/

	return std::string(result)+eventstr;
}

std::string CTCPTracker::EventStr( TTrackerEvent event )
{

    switch ( event )
    {

        case TE_START:
        return std::string( "&event=started" );

        case TE_STOP:
        return std::string( "&event=stopped" );

        case TE_COMPLETE:
        return std::string( "&event=completed" );

        default:
        return std::string( "" );
    }
}

void CTCPTracker::SetHash(char *hash)
{
	memcpy(m_Hash,hash,20);
}

void CTCPTracker::SetId(char *id)
{
	memcpy(m_Id,id,20);
}

bool CTCPTracker::SendHttpGetRequest(
    const char* shost, const char* sfile, const char* auth )
{

    char outbuf[ 1024 ] = {0};
    char buf[ 512 ] = {0};


    sprintf( buf, "GET %s HTTP/1.1\r\n", sfile );
    strcpy( outbuf, buf );

	sprintf( buf, "HOST: %s\r\n", shost);
	strcat( outbuf, buf );

    strcat( outbuf, "User-Agent: Mozilla/4.0(compatible;MSIE 5.00;Windows 98)\r\n" );

    strcat( outbuf, "Accept: */*\r\n" );

    if ( strlen( auth ) > 0 )
    {
        strcat( outbuf, "Authorization: Basic " );
        strcat( outbuf, auth );
        strcat( outbuf, "\r\n" );

    }

    strcat( outbuf, "Connection: close\r\n\r\n" );

	//syslog( outbuf );

	SetBlock(true);

#ifdef WIN32
    send ( m_hSocket, ( char* ) (outbuf), strlen(outbuf) , 0 /*flags*/ );
#else
    send ( m_hSocket, ( void* ) (outbuf), strlen(outbuf) , 0 /*flags*/ );
#endif

	SetBlock(false);
	return true;
}

TTrackerEvent CTCPTracker::GetCurrentEvent()
{

	return m_CurEvent;

}
//
//void CTCPTracker::OutMsg(wchar_t *msg, _MSGTYPE type)
//{
//	m_pParent->GetStorage()->LogMsg(msg,0,type);
//}

void CTCPTracker::Stop()
{

}


void CTCPTracker::SetEvent(TTrackerEvent event)
{
	m_CurEvent=event;
}

int CTCPTracker::GetInterval()
{
	int interval=1;
	for(int i=0;i<m_nFailCount;i++)
	{
		interval*=2;
	}
	return interval*10+rand()%5;
}

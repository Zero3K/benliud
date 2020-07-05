/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#include "../include/UPnpNatFinder.h"
#include "../include/UPnpNatFinderHandler.h"

#include <Dealer.h>
#include <Tools.h>

#if defined( WINCE)
#include <windows.h>
#elif defined( WIN32)
#include <winsock2.h>
#include <errno.h>
#include <assert.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#endif


extern UINT	gStateCode;	//系统状态码

//extern void syslog(std::string info);

#define HTTP_OK "200 OK"

#define HTTPMU_HOST_PORT 1900
#define MAX_DISCOVERY_RECEIVE_SIZE 400
#define SEARCH_REQUEST_STRING "M-SEARCH * HTTP/1.1\r\n"			\
                              "HOST: 239.255.255.250:1900\r\n"	\
                              "ST:upnp:rootdevice\r\n"			\
                              "MAN:\"ssdp:discover\"\r\n"		\
                              "MX: 3\r\n\r\n"

// class CUPnpNatFinder
CUPnpNatFinder::CUPnpNatFinder()
: _eventHandler( NULL )
        , _discoverTimerID( 0 )
        , _found( false )
{
    m_nTryTimes = 0;
	
}

CUPnpNatFinder::~CUPnpNatFinder()
{}


void CUPnpNatFinder::setEventHandler( CUPnpNatFinderHandler* handler )
{
    _eventHandler = handler;
}

void CUPnpNatFinder::Start()
{
    Discover();
}

void CUPnpNatFinder::Stop()
{
	Close();

}

void CUPnpNatFinder::Discover()
{
	CreateSock();

#ifdef WIN32

    bool val = true;

    if ( 0 != setsockopt( m_hSocket, SOL_SOCKET, SO_BROADCAST, ( char* ) & val, sizeof( bool ) ) )
    {
        //		printf("setsockopt broadcast fail\n");
    }

#endif

	std::vector<std::string>::iterator it;

	//每次探索时建立广播地址表可以适应动态变化的本地地址
	MakeIpList();  //make it every time may adopt the dynamic changing local ip

	for(it=m_IpList.begin();it!=m_IpList.end();it++)
	{
		std::string bip = (*it);

		std::string sendBuffer = SEARCH_REQUEST_STRING;

		Send( (char*)sendBuffer.data(), sendBuffer.size(), inet_addr( bip.c_str() ),htons( HTTPMU_HOST_PORT ));
 
	}
	
	maskRead( true );

    if ( _discoverTimerID != 0 )
    {
        RemoveTimer( _discoverTimerID );
    }

    _discoverTimerID = AddTimer( 5000 , true);

}

void CUPnpNatFinder::ParseReponse( std::string& content, int len )
{

	
//parse the content line by line!

	size_t pos_1,pos_2;
	pos_1=pos_2=0;

	int ncode=0;
	std::string location;

	while((pos_2=content.find('\n',pos_1))!=std::string::npos)
	{
		//found the pos2
		std::string line=content.substr(pos_1,pos_2-pos_1);
		pos_1=pos_2+1; //reset the pos_1
		Tools::TrimStringRight(line,"\r");

#ifdef WIN32
		if( line.length() > 8 && _stricmp(line.substr(0,7).c_str(), "HTTP/1.") ==0)
#else
		if( line.length() > 8 && strcasecmp(line.substr(0,7).c_str(), "HTTP/1.") ==0)		
#endif		
			
		{
			if(line.length() < 12) return; //too short

			std::string code=line.substr(9,3);

			ncode=atoi(code.c_str());
		}
#ifdef WIN32		
		else if( line.length() > 10 && _stricmp(line.substr(0,9).c_str(), "LOCATION:")==0)
#else
		else if( line.length() > 10 && strcasecmp(line.substr(0,9).c_str(), "LOCATION:")==0)		
#endif		
		{
			location=line.substr(10);
			Tools::TrimAllSpace(location);
		}
		else
		{
			continue;
		}

	}

	if(ncode/100 == 2 && !location.empty())
	{
		//success
		_found=true;

		if(_discoverTimerID!=0)
		{
			RemoveTimer(_discoverTimerID);
			_discoverTimerID=0;
		}
		
		gStateCode=31; //found a upnp device Description Url response
		_eventHandler->OnGetDescriptionUrl( location );
		
	}

}

void CUPnpNatFinder::OnRead()
{

	char buffer[4096*4];

    int ret = 0;
	unsigned int iip;
	unsigned short iport;

	ret=Recv(buffer, 4096*4-1, iip, iport);

    if ( ret > 0 )
    {
        buffer[ ret ] = 0;
		std::string content=buffer;
        ParseReponse( content, ret );
    }
	else
	{
		return;
	}

	Close();

	_discoverTimerID=0;

    return;
}

void CUPnpNatFinder::OnWrite()
{
    return ;
}

void CUPnpNatFinder::OnTimer( unsigned int id )
{
	CUDPClientSock::OnTimer(id);

    if ( id == _discoverTimerID )
    {
		Close(); //close old socket.
		_discoverTimerID=0;
		return;
    }
}

//works ok!
std::string CUPnpNatFinder::GetLocalHost()
{
    //get localhost and ip

    char name[ 256 ];
    std::string ip;

#ifdef WIN32

    PHOSTENT hostinfo;
#else

    struct hostent *hostinfo;
#endif

    if ( gethostname ( name, sizeof( name ) ) == 0 )
    {
        if ( ( hostinfo = gethostbyname( name ) ) != NULL )
        {

            ip = inet_ntoa ( *( struct in_addr * ) * hostinfo->h_addr_list );
        }
    }

    return ip;
}

void CUPnpNatFinder::MakeIpList()
{
    //make the ip we should broadcast.
    //std::string ip = GetLocalHost();

	m_IpList.clear();

    char name[ 256 ];
    std::string ip;

#ifdef WIN32
    PHOSTENT hostinfo;
#else
    struct hostent *hostinfo;
#endif

    if ( gethostname ( name, sizeof( name ) ) == 0 )
    {
        if ( ( hostinfo = gethostbyname( name ) ) != NULL )
        {
			sockaddr_in sa;
			for (int nAdapter=0; hostinfo->h_addr_list[nAdapter]; nAdapter++) 
			{
				if(hostinfo->h_addrtype==AF_INET6) continue;

				memcpy ( &sa.sin_addr.s_addr, hostinfo->h_addr_list[nAdapter],hostinfo->h_length);
				ip=inet_ntoa(sa.sin_addr);

				if ( IsInnerIp( ip ) )
				{
					std::string::size_type pos = ip.rfind( '.' );
					ip = ip.substr( 0, pos ) + ".255";
					m_IpList.push_back( ip ); // 192.168.0.255
				}

			}

        }
    }

    m_IpList.push_back( "239.255.255.250" ); //only standard broadcast
    m_IpList.push_back( "255.255.255.255" );
}


bool CUPnpNatFinder::IsInnerIp( std::string ip )
{
    /*
    A脌脿碌脴脰路拢潞10.0.0.0隆芦10.255.255.255 
     
    B脌脿碌脴脰路拢潞172.16.0.0隆芦172.31.255.255 
     
    C脌脿碌脴脰路拢潞192.168.0.0隆芦192.168.255.255
    */

	if ( ip=="") return true;
		
    if ( ip.substr( 0, 3 ) == "10." )
        return true;
	
    //if ( ip.substr( 0, 7 ) == "172.16." )
    //    return true; //TODO : fix it 172.17~31

	if( ip.substr(0,4) == "172.")
	{
		std::string sec= ip.substr(4,3);
		if(sec[3]!= '.') return false; //like 172.168.x.x
		
		int mid=atoi(ip.substr(4,2).c_str());
		if(mid>=16 && mid<=31) return true;
		else return false;
	}

    if ( ip.substr( 0, 8 ) == "192.168." )
    {
		return true;
	}
	
	if(ip.substr(0,8)=="169.254.")
	{
		return true;
	}

    return false;
}


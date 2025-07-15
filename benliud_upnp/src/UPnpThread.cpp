/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// UPnpThread.cpp: implementation of the CUPnpThread class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/UPnpThread.h"
#include "../include/UPnpNat.h"

#include <Tools.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include <string>
//extern void syslog(std::string s);

#if !defined( WIN32) && !defined(WINCE)
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#endif


//struct in_addr BIN_IPV4_ADDR_LOOPBACK = {127, 0, 0, 1};
//struct in6_addr BIN_IPV6_ADDR_LOOPBACK =   {   0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x0,
//                                             0x0, 0x1 };


extern UINT	gStateCode;	//系统状态码

CUPnpThread::CUPnpThread()
{
	m_pDealer=NULL;
	m_pUPnpNat=NULL;
	m_bStop=false;
}

CUPnpThread::~CUPnpThread()
{
	if(m_pUPnpNat) delete m_pUPnpNat;
	if(m_pDealer) delete m_pDealer;

}

bool CUPnpThread::Start()
{

#if  defined(WIN32)||defined(WINCE)
    WSADATA wsadata;
    WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
#endif
	//check if local ip, if not ,don't start UPNP module

	if(FindLocalForExternIP(m_sExternIP))
	{
		OutputDebugString(L"found extern ip at local");

		gStateCode=10; //got ip at local
		m_bInnerIP=false;
		m_bGotIP=true; //the m_sExternIP is extern ip

		m_pDealer=new SockLib::CDealer;
		m_pDealer->AddTimer(this,60*1000); //we need to check ip changes.

		return Run(false); 
	}
	else
	{
		
		OutputDebugString(L"found no extern ip at local, starting upnp.");

		gStateCode=20;

		m_bStop=false;
		m_bGotIP=false; //the m_sExternIP isn't extern ip
		m_bInnerIP=true;
		m_pDealer=new SockLib::CDealer;
		m_pUPnpNat=new CUPnpNat(this);
		m_pUPnpNat->SetDealer(m_pDealer);
		m_pUPnpNat->Start();
		m_pUPnpNat->GetExternIp();  //get extern ip right now
		
		return Run(false);
	}

}

void CUPnpThread::Stop()
{
	m_bStop=true;
	Wait();

	if(m_pUPnpNat) {
		m_pUPnpNat->Stop();
		delete m_pUPnpNat;
		m_pUPnpNat=NULL;
	}

	if(m_pDealer) {
		delete m_pDealer;
		m_pDealer=NULL;
	}
}

void CUPnpThread::Entry()
{

	while(!m_bStop)
	{
		m_pDealer->Dispatch();
	}

}

bool CUPnpThread::IsInnerIP( std::string ip )
{
    /*
    AÀàµØÖ·£º10.0.0.0¡«10.255.255.255 
     
    BÀàµØÖ·£º172.16.0.0¡«172.31.255.255 
     
    CÀàµØÖ·£º192.168.0.0¡«192.168.255.255

	169.254.x.x
    */

	if ( ip=="") return true;
		
    if ( ip.substr( 0, 3 ) == "10." )
        return true;
	
	if ( ip.substr( 0, 2 ) == "0.")
		return true;

	if ( ip.substr( 0, 4) == "127.")
		return true; //loop ip

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

	if( ip.substr(0,8) == "169.254.")
	{
		return true;
	}
	
    return false;
}


void CUPnpThread::NoticeExternIP(std::string ip)
{
	m_bGotIP=true;
	m_sExternIP=ip;
}

bool CUPnpThread::GetExternIP(char *ipbuf)
{
	if(!m_bGotIP) return false;
	if(ipbuf==NULL) return false;

	strcpy(ipbuf,m_sExternIP.c_str());
	return true;
}

void CUPnpThread::addPortMap(unsigned int port, std::string protocol)
{

	if(!m_bInnerIP||m_pUPnpNat==NULL) 
	{
		return;
	}
	m_pUPnpNat->addPortMapping(port,protocol);
}

void CUPnpThread::removePortMap( unsigned int port, std::string protocol )
{
	if(!m_bInnerIP||m_pUPnpNat==NULL) return;
	m_pUPnpNat->removePortMapping(port,protocol);
}


bool CUPnpThread::IsLoopIp(std::string ip)
{
	return ip.substr( 0, 4) == "127.";
}

bool CUPnpThread::IsExternIp(std::string ip)
{
	if(IsLoopIp(ip) ) return false;
	if(IsInnerIP(ip)) return false;

	return true;
}

//this function is different in windows and linux
bool CUPnpThread::FindLocalForExternIP(std::string &xip)
{

#if defined(WIN32)||defined(WINCE)

    char name[ 256 ];
 

    PHOSTENT hostinfo;

    if ( gethostname ( name, sizeof( name ) ) == 0 )
    {
        if ( ( hostinfo = gethostbyname( name ) ) != NULL )
        {
			sockaddr_in sa;

			for (int nAdapter=0; hostinfo->h_addr_list[nAdapter]; nAdapter++) 
			{
				if(hostinfo->h_addrtype==AF_INET6) continue;
				memcpy ( &sa.sin_addr.s_addr, hostinfo->h_addr_list[nAdapter],hostinfo->h_length);
				xip=inet_ntoa(sa.sin_addr);

				if(IsExternIp(xip)) 
				{
					return true;
				}

			}

        }
    }

    return false;

#else

	struct ifaddrs *ifbase, *ifap;
	struct sockaddr_in *addr4;
	char buf[NI_MAXHOST];    
	std::string ip;

    if(getifaddrs(&ifbase)) 
	{        
		return false;
	}

	for( ifap = ifbase; ifap != NULL; ifap=ifap->ifa_next)
	{
		if (ifap->ifa_addr==NULL) continue;
		if ((ifap->ifa_flags & IFF_UP) == 0) continue;
		if (AF_INET != ifap->ifa_addr->sa_family) continue; //only AF_INET, ignore AF_INET6 address
		addr4 = (struct sockaddr_in *)ifap->ifa_addr;
		ip=inet_ntop(AF_INET, (void*)(&addr4->sin_addr),buf, NI_MAXHOST );
		if(IsExternIp(ip)) 
		{
			xip=ip;
			freeifaddrs(ifbase);
			return true;
		}
	}

	return false;

#endif
}

//only used with check local extern ip changes
void CUPnpThread::OnTimer(unsigned int id)
{
	//std::string old=m_sExternIP;
	//if(FindLocalForExternIP(m_sExternIP))
	//{
	//	if(old!=m_sExternIP)
	//	{
	//		
	//		SendIPEvent();
	//	}
	//}
	
	FindLocalForExternIP(m_sExternIP);
}


/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

// DNSBuffer.cpp: implementation of the CDNSBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/DNSBuffer.h"

#ifdef WIN32
#pragma warning (disable: 4786)
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h> 
#elif defined(WINCE)
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib,"ws2.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDNSBuffer::CDNSBuffer()
{
#if defined(WINCE)||defined(WIN32)
		WSADATA wsadata;
		WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
#endif
}

CDNSBuffer::~CDNSBuffer()
{

}

bool CDNSBuffer::GetServerIP(std::string server, std::string &ip)
{
	std::map<std::string,std::string>::iterator it;
	it=m_Buffer.find(server);

	if(it!=m_Buffer.end())
	{
		ip=it->second;
		return true;
	}

//we parser the host and return the ip

	struct hostent *he;
	he=gethostbyname(server.c_str());
	if(he==NULL) return false;

	if ( he->h_addrtype != AF_INET && 
		he->h_addrtype != AF_INET6 )
			return false;

	ip = inet_ntoa( *(( struct in_addr* ) ( he->h_addr )) );

	m_Buffer[server]=ip;

	return true;
}




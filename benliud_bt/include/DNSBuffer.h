/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// DNSBuffer.h: interface for the CDNSBuffer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DNS_BUFFER_H
#define _DNS_BUFFER_H

#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include <map>
#include <string>

class CDNSBuffer  
{
public:
	bool GetServerIP(std::string server, std::string& ip);
	CDNSBuffer();
	virtual ~CDNSBuffer();

protected:
	std::map<std::string, std::string> m_Buffer;
};

#endif




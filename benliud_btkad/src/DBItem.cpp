/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// DBItem.cpp: implementation of the CDBItem class.
//
//////////////////////////////////////////////////////////////////////


#if defined(WINCE)
#include <windows.h>
#elif defined( WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include "../include/DBItem.h"

#include <memory.h>

//10分钟数据保持期
const unsigned int MAX_AGE = 10 * 60 * 1000;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBItem::CDBItem()
{
	memset(item,0,6);
	intime=GetTickCount();
}

CDBItem::CDBItem(const unsigned char* ip_port)
{
	memcpy(item,ip_port,6);
	intime=GetTickCount();
}
CDBItem::CDBItem(unsigned int iip, unsigned short iport)
{
	memcpy(item,&iip,4);
	memcpy(item+4,&iport,2);
	intime=GetTickCount();
}


CDBItem::~CDBItem()
{

}

bool CDBItem::IsExpired(unsigned int now) const
{
	return now - intime > MAX_AGE;
}

CDBItem& CDBItem::operator = (const CDBItem & other)
{
	memcpy(item, other.item, 6);
	intime=other.intime;
	return *this;
}

bool CDBItem::operator ==( const CDBItem& other)
{
	return memcmp(item,other.item,6)==0;
}

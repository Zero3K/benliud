
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#if defined(WIN32)||defined(WINCE)
#include <winsock2.h>
#include <windows.h>
#endif

#include "../include/KBucketEntry.h"


CKBucketEntry::CKBucketEntry()
{
	m_nFailed=0;
	m_nLastActive=GetTickCount();
}

CKBucketEntry::~CKBucketEntry()
{
}

CKBucketEntry::CKBucketEntry(const SockLib::TInetAddr4 & addr,const BTDHTKey & id)
{
	m_Addr=addr;
	m_NodeKey=id;
	m_nLastActive=GetTickCount();
}

CKBucketEntry::CKBucketEntry(const CKBucketEntry & other)
{
	m_Addr=other.GetAddress();
	m_NodeKey=other.GetKey();
	m_nLastActive=GetTickCount();
}	


CKBucketEntry& CKBucketEntry::operator = (const CKBucketEntry & other)
{
	m_Addr=other.GetAddress();
	m_NodeKey=other.GetKey();
	return *this;
}

bool CKBucketEntry::operator == (const CKBucketEntry & other) const
{
	return m_NodeKey==other.GetKey();
}


bool CKBucketEntry::isGood() const
{
	return GetTickCount()-m_nLastActive < 15*60*1000;
}

bool CKBucketEntry::isQuestionable() const
{
	return GetTickCount()-m_nLastActive >= 15*60*1000;
}

bool CKBucketEntry::isBad() const
{
	return m_nFailed > 2;
}


void CKBucketEntry::hasResponded()
{
	m_nLastActive=GetTickCount();
	m_nFailed=0;
}



unsigned int CKBucketEntry::GetLastActive()
{
	return m_nLastActive;
}

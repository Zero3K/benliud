/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _KBUCKETENTRY_H
#define _KBUCKETENTRY_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif


#include "BTDHTKey.h"
#include <Tools.h>
#include <Sock.h>
class CKBucketEntry
{
	SockLib::TInetAddr4 m_Addr;
	BTDHTKey m_NodeKey;
	unsigned int m_nLastActive;
	unsigned int m_nFailed;

public:
	unsigned int GetLastActive();
	
	CKBucketEntry();
	
	CKBucketEntry(const SockLib::TInetAddr4 & addr,const BTDHTKey & id);
	
	CKBucketEntry(const CKBucketEntry & other);
	
	virtual ~CKBucketEntry();
	
	CKBucketEntry & operator = (const CKBucketEntry & other);
	
	bool operator == (const CKBucketEntry & entry) const;
	
	const SockLib::TInetAddr4 & GetAddress() const {return m_Addr;}
	
	const BTDHTKey& GetKey() const {return m_NodeKey;}
	
	bool isGood() const;
	
	bool isQuestionable() const;

	bool isBad() const;

	void hasResponded();

	void requestTimeout() {m_nFailed++;}

};

#endif

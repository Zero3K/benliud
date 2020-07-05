/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// ClosestNodeStore.h: interface for the CClosestNodeStore class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CLOSESTNODESTORE_H
#define _CLOSESTNODESTORE_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "KBucketEntry.h"
#include <map>

class CClosestNodeStore  
{
	typedef std::map<BTDHTKey,CKBucketEntry> TKBucketEntryMap;
public:
	bool IsNearer(BTDHTKey& key);
	void NoSelf(bool noself=true);
	void InsertNode(CKBucketEntry& entry);
	CClosestNodeStore(BTDHTKey& basepoint,int maxresult);
	virtual ~CClosestNodeStore();
	unsigned int GetCount() const {return m_ResultMap.size();}
	const BTDHTKey & GetBaseKey() const {return m_BasePoint;}
	
	typedef TKBucketEntryMap::iterator Itr;
	typedef TKBucketEntryMap::const_iterator CItr;
	
	Itr begin() {return m_ResultMap.begin();}
	Itr end() {return m_ResultMap.end();}
	
	CItr begin() const {return m_ResultMap.begin();}
	CItr end() const {return m_ResultMap.end();}

protected:
	int m_MaxResult;
	BTDHTKey m_BasePoint;  //used to cal distance
	TKBucketEntryMap m_ResultMap;
	bool m_bNoSelf;
};

#endif 


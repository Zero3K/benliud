/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _NEARERNODESTORE_H
#define _NEARERNODESTORE_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "KBucketEntry.h"
#include <map>

class CNearerNodeStore  
{
	typedef std::map<BTDHTKey,CKBucketEntry> TKBucketEntryMap;
public:
	CNearerNodeStore(BTDHTKey& basepoint,int maxresult);
	virtual ~CNearerNodeStore();
	void NoSelf(bool noself=true);
	void InsertNode(CKBucketEntry& entry);

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

#endif // !defined(AFX_NEARERNODESTORE_H__173A2882_6DE0_43B4_9B5E_A82CE6631E71__INCLUDED_)

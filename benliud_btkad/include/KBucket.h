/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// KBucket.h: interface for the CKBucket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _KBUCKET_H
#define _KBUCKET_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <list>

#include "KBucketEntry.h"
#include <Tools.h>
typedef std::list<CKBucketEntry> TKBucketEntryList;

class CDHTNode;
class CClosestNodeStore;
class CKBucket  
{
public:
	void SetSlotLimit(int limit);
	bool IsSlotFull();
	bool IsContainAddr(SockLib::TInetAddr4& addr);
	int GetItemCount();
	void GetAllGoodNodeData(std::string &data);
	void GetAllNodeData(std::string &data);
	bool GetEntryAddr(BTDHTKey& key, SockLib::TInetAddr4& addr);
	void UpdateNoResponse(CKBucketEntry& node);
	bool NeedRefresh();
	void GetClosestNodes(CClosestNodeStore &closest);
	CKBucket(CDHTNode* parent);
	virtual ~CKBucket();
	void Update(CKBucketEntry& node);
	// See if this bucket contains an entry
	bool IsContain(const CKBucketEntry & entry) const;
	bool CleanBadEntry();
	void CheckQuestionableAndReplace(const CKBucketEntry & replace);
	void GetClosestGoodNodes(CClosestNodeStore &closest);
protected:
//	TKBucketEntryList m_PingingList; //pinging entry
	TKBucketEntryList m_WaitingList; //waiting to insert to EntryList for no room
	TKBucketEntryList m_EntryList;
	unsigned int m_nLastModify;
	unsigned int m_nSlotLimit;
	bool m_bRefreshing;
	CDHTNode* m_pParent;
};

#endif

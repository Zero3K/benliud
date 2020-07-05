/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// KBucket.cpp: implementation of the CKBucket class.
//
//////////////////////////////////////////////////////////////////////

#if defined(WINCE)
#include <windows.h>
#elif defined(WIN32)
#include <winsock2.h>
#include <windows.h>
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

#include "../include/KBucket.h"
#include "../include/DHTNode.h"
#include "../include/ClosestNodeStore.h"
#include "../include/DHTThread.h"

//#define BUCKET_SLOT (6) //8 is bittorrent protocol suggest

#define BUCKET_REFRESH_INTERVAL (5*60*1000)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKBucket::CKBucket(CDHTNode* parent)
{	
	m_pParent=parent;
	m_nSlotLimit=6;
	m_nLastModify=GetTickCount();
}

CKBucket::~CKBucket()
{

}

//return true means a new node added
void CKBucket::Update(CKBucketEntry& node)
{
	//put last seen at the tail
	TKBucketEntryList::iterator it ;

	for(it=m_EntryList.begin();it!=m_EntryList.end();it++)
	{
		// If in the list, move it to the end
		if(*it==node)
		{
			//m_pParent->m_pParent->LogMsg(L"old exists update",MSG_INFO);
			m_EntryList.erase(it);
			m_EntryList.push_back(node);
			m_nLastModify = GetTickCount();
			return ;
		}
	}

	//check the room
	if(m_EntryList.size() >= m_nSlotLimit) 
	{//no room, check if any node bad
		if(CleanBadEntry())
		{
			//m_pParent->m_pParent->LogMsg(L"replace bad update",MSG_WARNNING);

			m_EntryList.push_back(node);
			//m_nLastModify = GetTickCount(); //CleanBadEntry() did it
		}
		else
		{//no bad entry to replace, so check the questionable entry
			//m_pParent->m_pParent->LogMsg(L"check question update",MSG_WARNNING);
			CheckQuestionableAndReplace(node);
		}
	} 
	else
	{//have room ,just insert
		//m_pParent->m_pParent->LogMsg(L"insert update",MSG_INFO);

		m_EntryList.push_back(node);
		m_nLastModify = GetTickCount();
	}

}

bool CKBucket::CleanBadEntry()
{
	TKBucketEntryList::iterator it ;
	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if ((*it).isBad())
		{
			m_EntryList.erase(it);
			m_nLastModify = GetTickCount();
			return true;
		}
	}
	return false;
}

bool CKBucket::IsContain(const CKBucketEntry & entry) const
{
	TKBucketEntryList::const_iterator it;

	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if((*it)==entry) return true;
	}

	return false;
}

void CKBucket::CheckQuestionableAndReplace(const CKBucketEntry & replace)
{

	//TODO: waiting list will get very large if ping check return no error
	//, delete the timeout node when refresh!

	m_WaitingList.push_back(replace); // lets not have to many pending_entries calls going on

	bool haveping=false;
	TKBucketEntryList::iterator it;
	// we haven't found any bad ones so try the questionable ones
	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if ((*it).isQuestionable())
		{
			//give this task to our parent and just wait result.
			haveping=true;
			m_pParent->PingCheck((*it));
		}
	}

	if(!haveping) //all no question ,so abandon it
	{
		m_WaitingList.pop_back();
	}
}

void CKBucket::GetClosestNodes(CClosestNodeStore &closest)
{
	TKBucketEntryList::iterator it;
	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		closest.InsertNode((*it));
	}
}

bool CKBucket::NeedRefresh()
{

	unsigned int now = GetTickCount();
	if (m_nLastModify > now) //tick count circle
	{
		m_nLastModify = now;
		return false;
	}

	//check the waiting list ,delete the timeout waiting node
	TKBucketEntryList::iterator it;
	for(it=m_WaitingList.begin();it!=m_WaitingList.end();)
	{
		if( now - it->GetLastActive() > 25*1000) 
		{
			//wait too long time, delete it;
			m_WaitingList.erase(it++);
			continue;
		}

		it++;
	}

	return  !m_EntryList.empty() && (now - m_nLastModify >= BUCKET_REFRESH_INTERVAL);

}

void CKBucket::UpdateNoResponse(CKBucketEntry& node)
{
	//increase the fail times counter
	TKBucketEntryList::iterator it ;

	for(it=m_EntryList.begin();it!=m_EntryList.end();it++)
	{
		if(*it==node)
		{
			it->requestTimeout();

			//if this node is in pinging list and have waiting node ,replace it with waiting node
			if( !m_WaitingList.empty())
			{
				m_EntryList.erase(it);
				m_EntryList.push_back(m_WaitingList.front());
				m_WaitingList.pop_front();
				m_nLastModify = GetTickCount();
			}

			return;
		}
	}
}


void CKBucket::GetClosestGoodNodes(CClosestNodeStore &closest)
{
	TKBucketEntryList::iterator it;
	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if(it->isGood()) closest.InsertNode((*it));
	}
}

bool CKBucket::GetEntryAddr(BTDHTKey &key, SockLib::TInetAddr4 &addr)
{
	TKBucketEntryList::const_iterator it;

	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if(it->GetKey()==key) {
			addr=it->GetAddress();
			return true;
		}
	}

	return false;
}

void CKBucket::GetAllGoodNodeData(std::string &data)
{
	TKBucketEntryList::iterator it;
	for(it=m_EntryList.begin();it!=m_EntryList.end();it++)
	{
		if(it->isGood())
		{
			SockLib::TInetAddr4 addr=it->GetAddress();

			data.append((const char*)(&addr.iip),4);
			data.append((const char*)(&addr.iport),2);

		}
	}
}

void CKBucket::GetAllNodeData(std::string &data)
{
	TKBucketEntryList::iterator it;
	for(it=m_EntryList.begin();it!=m_EntryList.end();it++)
	{
		SockLib::TInetAddr4 addr=it->GetAddress();
		data.append((const char*)(&addr.iip),4);
		data.append((const char*)(&addr.iport),2);
	}
}

int CKBucket::GetItemCount()
{
	return m_EntryList.size();
}

bool CKBucket::IsContainAddr(SockLib::TInetAddr4 &addr)
{
	TKBucketEntryList::const_iterator it;

	for (it = m_EntryList.begin();it != m_EntryList.end();it++)
	{
		if(it->GetAddress()==addr) return true;
	}

	return false;
}

bool CKBucket::IsSlotFull()
{
	if(m_EntryList.size() < m_nSlotLimit) return false;

	return !CleanBadEntry();
}


void CKBucket::SetSlotLimit(int limit)
{
	assert(limit>0 && limit < 9);
	m_nSlotLimit=limit;
}

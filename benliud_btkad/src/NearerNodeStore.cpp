/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// NearerNodeStore.cpp: implementation of the CNearerNodeStore class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/NearerNodeStore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNearerNodeStore::CNearerNodeStore(BTDHTKey& basepoint,int maxresult)
{
	m_BasePoint=basepoint;
	m_MaxResult=maxresult;
	m_bNoSelf=true; //no self result include
}

CNearerNodeStore::~CNearerNodeStore()
{

}

void CNearerNodeStore::InsertNode(CKBucketEntry &entry)
{
	if(m_bNoSelf && entry.GetKey()==m_BasePoint) 
	{
		return;
	}
	//check if closest node, if yes, insert it in result
	if(m_ResultMap.size()<m_MaxResult)
	{
		//BTDHTKey dis = BTDHTKey::distance(m_BasePoint,entry.GetKey());
		//m_ResultMap.insert(std::make_pair(dis,entry));
		m_ResultMap.insert(std::make_pair(m_BasePoint-entry.GetKey(),entry));
		return;
	}
	else
	{
		//BTDHTKey dis = BTDHTKey::distance(m_BasePoint,entry.GetKey());
		BTDHTKey dis = m_BasePoint-entry.GetKey();
		const BTDHTKey &max = m_ResultMap.rbegin()->first;
		if (dis < max)
		{
			//m_ResultMap.insert(std::make_pair(dis,entry));
			m_ResultMap.insert(std::make_pair(dis,entry));
			m_ResultMap.erase(max);
		}
	}

}

void CNearerNodeStore::NoSelf(bool noself)
{
	m_bNoSelf=noself;
}


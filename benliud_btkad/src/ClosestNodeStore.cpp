// ClosestNodeStore.cpp: implementation of the CClosestNodeStore class.
//
//////////////////////////////////////////////////////////////////////


#include "../include/ClosestNodeStore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CClosestNodeStore::CClosestNodeStore(BTDHTKey& basepoint,int maxresult)
{
	m_BasePoint=basepoint;
	m_MaxResult=maxresult;
	m_bNoSelf=true; //no self result include
}

CClosestNodeStore::~CClosestNodeStore()
{

}

void CClosestNodeStore::InsertNode(CKBucketEntry &entry)
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
			m_ResultMap.insert(std::make_pair(dis,entry));
			m_ResultMap.erase(max);
		}
	}

}

void CClosestNodeStore::NoSelf(bool noself)
{
	m_bNoSelf=noself;
}

//判断这个KEY距离basepoint是否比现在的其中一个要近？
bool CClosestNodeStore::IsNearer(BTDHTKey &key)
{
	if(m_ResultMap.empty()) return true;

	BTDHTKey dis = m_BasePoint-key;
	const BTDHTKey &max = m_ResultMap.rbegin()->first; //first is distance
	return (dis <= max); //比距离最大的还小
}

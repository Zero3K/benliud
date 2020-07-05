/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// DHTNode.cpp: implementation of the CDHTNode class.
//
//////////////////////////////////////////////////////////////////////


#include "../include/DHTNode.h"
#include "../include/KBucket.h"
#include "../include/DHTThread.h"
#include "../include/TAntTask.h"
#include "../include/BTDHTKey.h"
#include "../include/ClosestNodeStore.h"
#include "../include/FrontServer.h"
#include <AutoLock.h>
#include <Dealer.h>
//#include <time.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern void syslog(std::string);

CDHTNode::CDHTNode(CDHTThread* parent)
{
	srand(GetTickCount());

	m_MyID.Random();  //set a random key

	for (int i = 0;i < 160;i++)
	{
		m_BucketArray[i] = NULL;
	}

	m_pParent=parent;
	m_RefreshTimerId=0;
	m_BootupCheckTimerId=0;
	m_GetPeersTimerId=0;
	m_nSlotLimit=6;
	m_bFindPeer=true;
	m_bAnnounce=true;
}

CDHTNode::~CDHTNode()
{
	for (int i = 0;i < 160;i++)
	{
		if (m_BucketArray[i] != NULL)
		{
			delete m_BucketArray[i];
		}
	}
}

//the bucket call this to check a node
void CDHTNode::PingCheck(CKBucketEntry &entry)
{
	BTDHTKey key=entry.GetKey();
	DoPingOnNode(entry.GetAddress().iip,entry.GetAddress().iport,key);
}

void CDHTNode::GetClosestNodes(CClosestNodeStore &closest)
{
	// go over all buckets 
	for (int i = 0;i < 160;i++)
	{
		if (m_BucketArray[i]!=NULL)
		{
			m_BucketArray[i]->GetClosestNodes(closest);
		}
	}


}

void CDHTNode::RefreshBucket()
{
	for (int i = 0;i < 160;i++)
	{
		
		if (m_BucketArray[i]!=NULL && m_BucketArray[i]->NeedRefresh())
		{

			BTDHTKey randkey = GetRandomKeyInBucket(i,m_MyID);

			CClosestNodeStore closest(randkey,8);
			m_BucketArray[i]->GetClosestNodes(closest);

			if(closest.GetCount()>0)
			{
				CClosestNodeStore::Itr it;
				for(it=closest.begin();it!=closest.end();it++)
				{
					SockLib::TInetAddr4& nodeaddr=(SockLib::TInetAddr4&)(it->second.GetAddress());
					BTDHTKey&  targetid=(BTDHTKey&)(it->second.GetKey());
					DoFindNodeOnNode(nodeaddr.iip,nodeaddr.iport,targetid,randkey);
				}
			}
		}
	}
}

// Generate a random key which lies in a certain bucket
// before bit b is the same as mykey, after bit b is random
BTDHTKey CDHTNode::GetRandomKeyInBucket(int b, BTDHTKey& mykey)
{
	//// first generate a random one
	//BTDHTKey r;
	//r.Random(); //set to a random key
	//
	//unsigned char* data = (unsigned char*)r.GetData();

	//make a random key
	unsigned char data[20];

	for(int k=0;k<20;k++)
	{
		data[k]=rand()%0xFF;
	}
	
	// before we hit bit b, everything needs to be equal to our_id

	unsigned char nb=b/8;
	
	for (unsigned char i = 0;i < nb;i++)
	{
		data[i] = *(mykey.GetData() + i);
	}
	
	
	// copy all bits of ob, until we hit the bit which needs to be different
	unsigned char ob = *(mykey.GetData() + nb);
	
	for (unsigned char j = 0;j < b % 8;j++)
	{
		if ((0x80 >> j) & ob)
			data[nb] |= (0x80 >> j);
		else
			data[nb] &= ~(0x80 >> j);
	}
	
	// if the bit b is on turn it off else turn it on
	if ((0x80 >> (b % 8)) & ob)
		data[nb] &= ~(0x80 >> (b % 8));
	else
		data[nb] |= (0x80 >> (b % 8));
	
	return BTDHTKey ((const char*)data);
	
}

unsigned char CDHTNode::FindBucket(BTDHTKey &id)
{
//Kademlia nodes store contact information about
//each other to route query messages. For each
//0 <= i < 160, every node keeps a list of
//<IP address; UDP port; Node ID> triples for nodes of
//distance between 2i and 2i+1 from itself.

	BTDHTKey distance = id-m_MyID;

	unsigned char ret=0xFF;

	for (int i = 0; i < 20; i++)
	{
		// get the byte
		unsigned char b = *(distance.GetData() + i);

		if (b == 0x00)
			continue;
		
		for (unsigned char j = 0;j < 8;j++)
		{
			if (b & (0x80 >> j))
			{

				ret = (19 - i)*8 + (7 - j);

				return ret; 
			}
		}
	}

	return ret;
}

void CDHTNode::OnTimer(unsigned int id)
{
	if(id==m_BootupCheckTimerId) 
	{
		if(!BootUp())
		{//用光了引导点

			if(GetItemCount()<6)
			{
				m_pParent->AddFixNodes(); //重新添加固定引导点
			}

		}

	}else if(id==m_RefreshTimerId){
		RefreshBucket(); //refresh bucket every 1 min
	}else if(id==m_GetPeersTimerId){
		DoGetPeersJob(); 
	}

}

void CDHTNode::Start()
{
	m_RefreshTimerId=m_pParent->GetDealer()->AddTimer(this,60*1000);
	m_GetPeersTimerId=m_pParent->GetDealer()->AddTimer(this,2*1000); //2 second check get_peers
	m_BootupCheckTimerId=m_pParent->GetDealer()->AddTimer(this,5*1000); //every 5s check boot up

}


void CDHTNode::DoGetPeersOnNode(unsigned int ip, unsigned short port, BTDHTKey& nodekey, BTDHTKey& target)
{

	TRequestTask task;
	task.response=0;
	//task.prio=7; //high priority
	task.prio=_GET_PEER_PRIO;
	task.destnode.iip=ip;
	task.destnode.iport=port;
	memcpy(task.peerkey,nodekey.GetData(),20);
	memcpy(task.targetkey,target.GetData(),20);
	task.tasktype=TSK_GET_PEERS;

	m_pParent->GetFrontServer()->DoRequest(task);
}

void CDHTNode::DoPingOnNode(unsigned int ip, unsigned short port, BTDHTKey& nodekey)
{

	TRequestTask task;
	task.response=0;
	//task.prio=6;//low priority
	task.prio=_PING_PRIO;
	task.destnode.iip=ip;
	task.destnode.iport=port;
	memcpy(task.peerkey,nodekey.GetData(),20);
	memset(task.targetkey,0,20);

	task.tasktype=TSK_PING;

	m_pParent->GetFrontServer()->DoRequest(task);
}

void CDHTNode::DoFindNodeOnNode(unsigned int ip, unsigned short port,  BTDHTKey& nodekey,  BTDHTKey& findkey)
{

	TRequestTask task;
	task.response=0;
	//task.prio=9;
	task.prio=_FIND_NODE_PRIO;
	task.destnode.iip=ip;
	task.destnode.iport=port;
	memcpy(task.peerkey,nodekey.GetData(),20);
	memcpy(task.targetkey,findkey.GetData(),20);
	task.tasktype=TSK_FIND_NODE;

	m_pParent->GetFrontServer()->DoRequest(task);
}

//every received response should call this to update our kbucket
void CDHTNode::Update(SockLib::TInetAddr4 &addr, BTDHTKey& key)
{
	unsigned char pos=FindBucket(key);

	if(pos>=160) 
	{
		return; //some wrong
	}
	
	if(m_BucketArray[pos]==NULL)
	{
		m_BucketArray[pos]=new CKBucket(this);
		m_BucketArray[pos]->SetSlotLimit(m_nSlotLimit);
	}

	CKBucketEntry entry(addr,key);
	m_BucketArray[pos]->Update(entry);

}

//every timeout task call this to update the failtime or replace the node with good node
//maybe this address havn't got the key or even not in the bucket, just ignore it.
//the key maybe empty key if in initial state
void CDHTNode::UpdateNoResponse(SockLib::TInetAddr4 &addr, BTDHTKey& key)
{
	unsigned char pos=FindBucket(key);

	if(pos>=160) {
		return; 
	}

	if(m_BucketArray[pos]==NULL) return; //should not happen, we can't build bucket for a invalid node

	CKBucketEntry entry(addr,key);
	m_BucketArray[pos]->UpdateNoResponse(entry);

}


//as a id key for map
std::string CDHTNode::GetPeerId(const SockLib::TInetAddr4 &addr)
{
	char buf[130];

	sprintf(buf,"%u:%u",addr.iip,addr.iport);

	return buf;

}

//return true is new node
bool CDHTNode::AddNewGotNode(SockLib::TInetAddr4 &addr,BTDHTKey& key)
{

	if(IsUsedAddr(addr))
	{
		return false;
	}
	else
	{
		//m_AllNodes[GetPeerId(addr)]=(unsigned int)time(NULL);
		m_AllNodes[GetPeerId(addr)]=GetTimeTick();

		//Update(addr,key);
		return true;
	}

}

bool CDHTNode::AddGetPeerTask(const char *infohash,bool announce,unsigned short tport)
{
		
	//just like add a task
	BTDHTKey hash(infohash);
	TTaskList::iterator it;

	m_TaskListMutex.Lock();
	for(it=m_TaskList.begin();it!=m_TaskList.end();it++)
	{
		if(it->key==hash) {
			it->announce=announce;
			it->port=tport;
			m_TaskListMutex.Unlock();
			return true;
		}
	}


	TDHTTask item;
	item.key=BTDHTKey(infohash);
	item.announce=announce;
	item.port=tport;
	item.nextget=0; //get it right now

	m_TaskList.push_back(item);

	m_TaskListMutex.Unlock();
	return true;

}


void CDHTNode::GetClosestGoodNode(CClosestNodeStore &store)
{
	for (int i = 0;i < 160;i++)
	{
		if (m_BucketArray[i] )
		{
			m_BucketArray[i]->GetClosestGoodNodes(store);
		}
	}

		
}


void CDHTNode::DoGetPeersJob()
{
	//这里可能不应该从路由表中的最近节点开始
	//路由表中的节点只是为了基本的路由，里面
	//的节点数量有限，一个插口最多只有8个节点
	//而我们不希望每次都从基础开始，所以应该
	//从活动节点池中去找8个最近的来开始。


	TTaskList::iterator it;

	ULONGLONG now;

	SYSTEMTIME st;
	FILETIME ft;
	::GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	now = ULONGLONG(ft.dwLowDateTime) + (ULONGLONG(ft.dwHighDateTime)<<32);
	
	SockLib::CAutoLock al(m_TaskListMutex);

	//choose the closest 8 node to get the peer value
	for(it=m_TaskList.begin();it!=m_TaskList.end();it++)
	{

		if(now < it->nextget) {
			continue;  //wait for nextget
		}
			
		BTDHTKey key=it->key;	//infohash

		CClosestNodeStore closest(key,8);


		//从节点池开始应该比从路由表要快，因为路由表里的节点最多也就200个
		//节点池可以存放所有见到的活跃点，可能存放几千个

		GetClosestGoodNodeInActivePool(closest);

		if( closest.GetCount() > 7 ) //at least we need 7 node to start get_peers traverl 
		{

			CClosestNodeStore::Itr it2;
			for(it2=closest.begin();it2!=closest.end();it2++)
			{
				SockLib::TInetAddr4& nodeaddr=(SockLib::TInetAddr4&)(it2->second.GetAddress());
				BTDHTKey&  targetid=(BTDHTKey&)(it2->second.GetKey());

				DoGetPeersOnNode(nodeaddr.iip,nodeaddr.iport,targetid,key);
			}
			
			it->nextget= GetTimeTick() + 60; //next update will be 1 min later
		}
		else
		{
			break;
		}

	}

}

bool CDHTNode::IsContain( BTDHTKey& key) 
{

	unsigned char pos=FindBucket(key);
	if(pos>=160) return false;

	if(m_BucketArray[pos]==NULL) return false;

	SockLib::TInetAddr4 addr;
	CKBucketEntry entry(addr,key);

	return m_BucketArray[pos]->IsContain(entry);
}

bool CDHTNode::GetEntryAddr(BTDHTKey &key, SockLib::TInetAddr4 &addr)
{
	unsigned char pos=FindBucket(key);
	if(pos>=160) return false;

	if(m_BucketArray[pos]==NULL) return false;

	return m_BucketArray[pos]->GetEntryAddr(key,addr);
}



bool CDHTNode::IsBucketEmpty()
{
	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i]!=NULL) return false;
	}

	return true;
}

bool CDHTNode::BootUp()
{

	//get the initial nodes from parent, insert into the map

	int count=0;

	BTDHTKey empty;
	SockLib::TInetAddr4 addr;

	for(int i=0;i<8;i++)
	{
		if(!m_pParent->GetInitNode(addr))
		{
			break;
		}

		if(!IsUsedAddr(addr))
		{
			count++;
			m_AllNodes[GetPeerId(addr)]=GetTimeTick();

			DoFindNodeOnNode(addr.iip,addr.iport,empty,m_MyID);
		}
	}
	
	return count>0;

}

void CDHTNode::RemoveGetPeersTask(char *infohash)
{
	//just like add a task
	BTDHTKey hash(infohash);

	TTaskList::iterator it;

	m_TaskListMutex.Lock();

	for(it=m_TaskList.begin();it!=m_TaskList.end();it++)
	{
		if(it->key==hash) {
			m_TaskList.erase(it);
			break;
		}
	}

	m_TaskListMutex.Unlock();

	return ;
}



void CDHTNode::GetAllGoodNodeData(std::string &data)
{
//campact all the node data to string
	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i]!=NULL)
		{
			m_BucketArray[i]->GetAllGoodNodeData(data);
		}
	}
}


void CDHTNode::GetAllNodeData(std::string &data)
{
//campact all the node data to string
	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i]!=NULL)
		{
			m_BucketArray[i]->GetAllNodeData(data);
		}
	}
}


void CDHTNode::DoAnnounceJob(std::string &tokenstr, std::string &infohash, unsigned int ip, unsigned short port)
{

	//if(!m_bAnnounce) return;


	//check if the task need announce
	BTDHTKey hash(infohash.data());

//检查是否是重复的申明

	TAnnounceRecord::iterator it;

	it=m_AnnounceRecord.find(hash);

	if(it!=m_AnnounceRecord.end())
	{
		TAnnounceList::iterator it2;
		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			if(it2->iip==ip) 
			{
				if(GetTickCount() - it2->tick < 5*60*1000 ) 
				{
					return;
				} else {
					it->second.erase(it2);
					break;
				}
			}
		}

	}



	TTaskList::iterator it2;

	m_TaskListMutex.Lock();

	for(it2=m_TaskList.begin();it2!=m_TaskList.end();it2++)
	{
		if(it2->key==hash ) 
		{

			if(it2->announce)
			{
				//yes ,announce it
				TRequestTask task;
				task.response=0;
				task.prio=8; //highest priority
				task.destnode.iip=ip;//got ip from addr
				task.destnode.iport=port; //got port from addr
				task.lport=it2->port;  //the task is listen on port
				task.token=tokenstr;
				memcpy(task.targetkey,infohash.data(),20);	//target is the infohash now
				task.tasktype=TSK_ANNOUNCE_PEER;

				m_pParent->GetFrontServer()->DoRequest(task);
			}

			break;

		}
	}

	m_TaskListMutex.Unlock();
	return ;
}

int CDHTNode::GetItemCount()
{
	int count=0;

	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i])
		{
			count+=m_BucketArray[i]->GetItemCount();
		}
	}

	return count;
}

//检查任务是否存在，任务可在中途被删除
bool CDHTNode::IsTaskExists(BTDHTKey &key)
{
	SockLib::CAutoLock al(m_TaskListMutex);

	TTaskList::iterator it;
	for(it=m_TaskList.begin();it!=m_TaskList.end();it++)
	{
		if(it->key==key) {
			return true;
		}
	}

	return false;
}

bool CDHTNode::IsContainAddr(SockLib::TInetAddr4 &addr)
{

	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i]!=NULL) 
		{
			if(m_BucketArray[i]->IsContainAddr(addr))
				return true;
		}
	}

	return false;
}

bool CDHTNode::IsUsedAddr(SockLib::TInetAddr4 &addr)
{

	return m_AllNodes.find(GetPeerId(addr))!=m_AllNodes.end();
	
}

bool CDHTNode::IsSlotFull(BTDHTKey &key)
{
	unsigned char pos=FindBucket(key);
	if(pos>=160) return true;

	if(m_BucketArray[pos]==NULL)
	{
		return false;
	}

	return m_BucketArray[pos]->IsSlotFull();
}

//记录find_node查找过的接点，避免反复find_node
bool CDHTNode::RecordNodeForFindNode(SockLib::TInetAddr4 &addr)
{
	TFindRecord::iterator it=m_FindNodes.find(addr.iip);
	if(it!=m_FindNodes.end()) return false;

	m_FindNodes[addr.iip]=addr.iport;
	return true;
}

bool CDHTNode::RecordNodeForGetPeers(SockLib::TInetAddr4 &addr)
{
	TFindRecord::iterator it=m_GetPeerNodes.find(addr.iip);
	if(it!=m_GetPeerNodes.end()) return false;

	m_GetPeerNodes[addr.iip]=addr.iport;
	return true;
}

//return : false= have old value, true= a new value
bool CDHTNode::AddActiveNode(BTDHTKey &key, SockLib::TInetAddr4 &addr)
{
	if(m_NodePool.find(key)!=m_NodePool.end()) return false;

	m_NodePool[key]=addr;
	return true;
}

void CDHTNode::RemoveActiveNode(BTDHTKey &key)
{
	m_NodePool.erase(key);
}

void CDHTNode::GetClosestGoodNodeInActivePool(CClosestNodeStore &store)
{
	TNodePool::const_iterator it;
	for(it=m_NodePool.begin();it!=m_NodePool.end();it++)
	{
		CKBucketEntry entry(it->second,it->first);
		store.InsertNode(entry);
		//store.InsertNode(CKBucketEntry(it->second,it->first));
	}
}

bool CDHTNode::IsNodeInActivePool(BTDHTKey &key)
{
	return (m_NodePool.find(key)!=m_NodePool.end());
}

void CDHTNode::GoodAnnouncePeersResponse(unsigned int ip, unsigned short port, BTDHTKey& key)
{
	//record it;
	TAnnounceRecord::iterator it;

	it=m_AnnounceRecord.find(key);

	if(it==m_AnnounceRecord.end())
	{
		TAnnounceList empty;
		_announce item;
		item.iip=ip;
		item.tick=GetTimeTick();
		empty.push_back(item);

		m_AnnounceRecord[key]=empty;
	}
	else
	{
		TAnnounceList::iterator it2;
		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			if(it2->iip==ip) {
				it2->tick=GetTickCount(); return;
			}
		}

		_announce item;
		item.iip=ip;
		item.tick=GetTimeTick();
		it->second.push_back(item);
	}
}

void CDHTNode::SetSlotLimit(int limit)
{
	m_nSlotLimit=limit;
	for(int i=0;i<160;i++)
	{
		if(m_BucketArray[i]!=NULL)
		{
			m_BucketArray[i]->SetSlotLimit(m_nSlotLimit);
		}
	}
}

//64bit second record
ULONGLONG CDHTNode::GetTimeTick()
{
	SYSTEMTIME st;
	FILETIME ft;
	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);
	return ULONGLONG(ft.dwLowDateTime) + (ULONGLONG(ft.dwHighDateTime)<<32);
}
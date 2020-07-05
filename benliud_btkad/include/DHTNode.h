/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// DHTNode.h: interface for the CDHTNode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DHTNODE_H
#define _DHTNODE_H


//my node object
#include "BTDHTKey.h"


class CKBucket;
class CClosestNodeStore;
class CKBucketEntry;
class CDHTThread;

#include <string>
#include <map>
#include <list>

#if defined(WINCE)
#include <windows.h>
#elif defined( WIN32)
#pragma warning(disable: 4786)
#include <winsock2.h>
#endif

#include <Mutex.h>
#include <TimerClient.h>
#include <Sock.h>

//the key is infohash, the list is the peers
typedef std::list<SockLib::TInetAddr4> TAddrList;

class TDHTTask
{
public:
	BTDHTKey key;
	bool announce;
	unsigned short port;
	ULONGLONG	nextget; //next get_peers tickcount //64位的秒数
};


typedef std::list<TDHTTask> TTaskList;

class CDHTNode : public SockLib::CTimerClient
{
	//曾经申明成功的记录，不用反复申明了
	struct _announce
	{
		unsigned int iip;
		ULONGLONG tick;
	};

	typedef std::map<std::string ,ULONGLONG> TAllNodeMap;
	typedef std::map<unsigned int,unsigned short> TFindRecord;

	typedef std::map<BTDHTKey, SockLib::TInetAddr4> TNodePool; //节点信息池，用于get_peers
	
	typedef std::list<_announce> TAnnounceList;
	typedef std::map<BTDHTKey, TAnnounceList> TAnnounceRecord;

public:
	void SetSlotLimit(int limit);
	bool IsNodeInActivePool(BTDHTKey& key);
	void GetClosestGoodNodeInActivePool(CClosestNodeStore& store);
	void RemoveActiveNode(BTDHTKey& key);
	bool AddActiveNode(BTDHTKey& key, SockLib::TInetAddr4& addr);
	bool RecordNodeForGetPeers(SockLib::TInetAddr4& addr);
	bool RecordNodeForFindNode(SockLib::TInetAddr4& addr);
	bool IsSlotFull(BTDHTKey& key);
	bool IsContainAddr(SockLib::TInetAddr4& addr);
	bool IsTaskExists(BTDHTKey& key);
	int GetItemCount();
	void DoAnnounceJob(std::string &tokenstr, std::string &infohash, unsigned int ip, unsigned short port);
	void GetAllGoodNodeData(std::string& data);
	void GetAllNodeData(std::string &data);
	void RemoveGetPeersTask(char* hash);
	bool GetEntryAddr(BTDHTKey& key, SockLib::TInetAddr4& addr);
	bool AddGetPeerTask(const char *infohash,bool announce,unsigned short tport);
	bool AddNewGotNode(SockLib::TInetAddr4& addr,BTDHTKey& key);
	void UpdateNoResponse(SockLib::TInetAddr4 &add, BTDHTKey& key);
	void Update(SockLib::TInetAddr4 &add, BTDHTKey& key);
	void DoFindNodeOnNode(unsigned int ip, unsigned short port, BTDHTKey& nodekey, BTDHTKey& findkey);
	void DoPingOnNode(unsigned int ip,unsigned short port, BTDHTKey& nodekey);
	void Start();
	void OnTimer(unsigned int id);
	unsigned char FindBucket( BTDHTKey& id);
	void RefreshBucket();
	void GetClosestNodes(CClosestNodeStore& closest);
	void PingCheck(CKBucketEntry& entry);
	CDHTNode(CDHTThread* parent);
	virtual ~CDHTNode();
	const BTDHTKey & GetMyID() const {return m_MyID;}
	void DoGetPeersOnNode(unsigned int ip,unsigned short port,BTDHTKey& nodekey,BTDHTKey& target);
	void GetClosestGoodNode(CClosestNodeStore& store);
	bool IsContain( BTDHTKey& key) ;
	void GoodAnnouncePeersResponse(unsigned int ip, unsigned short port, BTDHTKey& key);


private:
	BTDHTKey m_MyID;
	CKBucket* m_BucketArray[160]; //160 is for bittorrent, for ed2k is 128

protected:
	bool IsUsedAddr(SockLib::TInetAddr4& addr);
	bool BootUp();
	bool IsBucketEmpty();
	void DoGetPeersJob();
	ULONGLONG GetTimeTick();
	std::string GetPeerId(const SockLib::TInetAddr4& addr);
	BTDHTKey GetRandomKeyInBucket(int bucket, BTDHTKey& mykey);
private:
	CDHTThread*		m_pParent;
	unsigned int	m_RefreshTimerId;
	unsigned int	m_GetPeersTimerId;
	unsigned int	m_BootupCheckTimerId;
	int				m_nSlotLimit;
	bool			m_bFindPeer;  //use dht to find peer
	bool			m_bAnnounce;	//announce ourself in dht
	bool			m_bAsServer;	//Run as a server (can store other announce)


	TAllNodeMap	m_AllNodes;

	TFindRecord m_FindNodes;	//avoid find_node do on same address
	TFindRecord m_GetPeerNodes;	//avoid get_peers do on same address
	
	TNodePool	m_NodePool;		//节点池，只存放有效节点，和路由表不同

	SockLib::CMutex		m_TaskListMutex;
	TTaskList	m_TaskList;		//it's the get_peers task 

	TAnnounceRecord	m_AnnounceRecord;

	std::list<SockLib::TInetAddr4> m_InitialNodeList;
	
};

#endif

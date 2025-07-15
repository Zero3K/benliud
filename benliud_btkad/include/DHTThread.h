/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// DHTThread.h: interface for the CDHTThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DHTTHREAD_H
#define _DHTTHREAD_H
#ifdef WIN32
#pragma warning(disable: 4786)
#include <winsock2.h>
#endif
#include <ThreadBase.h>
#include <Mutex.h>

#include "BTDHTKey.h"
#include "ClosestNodeStore.h"
#include "../../benliud/include/callback_def.h"

// Add required typedefs
typedef void (*LOGBACK)(const char* msg);

#include <string>
#include <list>

//packet priority define
#define _ERROR_RES_PRIO (1)
#define _ANNOUNCE_RES_PRIO (2)
#define _FIND_NODE_RES_PRIO (3)
#define _GET_PEER_RES_PRIO (4)
#define _PING_RES_PRIO (5)

#define _FIND_NODE_PRIO (6)
#define _GET_PEER_PRIO (7)
#define _PING_PRIO (8)
#define _ANNOUNCE_PRIO (9)

//#define _FIND_NODE_PRIO (9)


class CDealer;
class CDHTNode;
class CDataBase;
class CDHTListener;
class CFrontServer;
class CDBItem;

typedef std::list<CDBItem> DBItemList;
class CDHTThread : public SockLib::CThreadBase
{
	friend class CDHTNode;
public:
	bool GetInitNode(SockLib::TInetAddr4& addr);
	void AddFixNodes();
	void SetSavePath(const char* path);
	void SetLogBack(LOGBACK callback);
	void SetEventBack(SERVICEEVENT callback);
	int DoPeerAnnounceTask(std::string& token, std::string& hash, SockLib::TInetAddr4& iaddr, int lport );
	void SetOptions(bool findpeer,bool announce,bool server,unsigned short level);
	int GetItemCount();
	void DoAnnounceTask(std::string& tokenstr, std::string& infohash, unsigned int ip, unsigned short port);

	int GetPeers(char* hash, int buflen, char* buf, int* total);
	void RemoveTask(char* hash);
	void AddInitialNodes(unsigned int ip, unsigned short port);
	void UpdateForRequest(SockLib::TInetAddr4& addr,BTDHTKey& nodekey);
	CFrontServer* GetFrontServer();
	CDataBase* GetDataBase();
	void GetClosestGoodNode(CClosestNodeStore &store);
	bool GetEntryAddr(BTDHTKey& key,SockLib::TInetAddr4& addr);
	bool IsContain(BTDHTKey& key);
	bool AddTask(const char* infohash,bool announce,unsigned port);
	unsigned short GetListenPort();
	void GoodAnnouncePeersResponse(unsigned int ip,unsigned short port, BTDHTKey& key);
	void ErrorAnnouncePeersResponse(unsigned int ip,unsigned short port);
	void GoodGetPeerResponse(unsigned int ip, unsigned short port, BTDHTKey &key, BTDHTKey& target, bool peers, std::string& peerStr);
	void NoResponse(unsigned int ip,unsigned short port,BTDHTKey& key);
	void GoodPingResponse(unsigned int ip,unsigned short port, BTDHTKey& key);
	void ErrorPingResponse(unsigned int ip,  unsigned short port,BTDHTKey& key);
	void ErrorFindNodeResponse(unsigned int ip,  unsigned short port,BTDHTKey& key);
	void GoodFindNodeResponse(unsigned int ip,  unsigned short port,BTDHTKey &key, std::string& peerStr,BTDHTKey &target);
	void ErrorGetPeersResponse(unsigned int ip,  unsigned short port, BTDHTKey& peerid);
	const BTDHTKey& GetMyID();

	SockLib::CDealer* GetDealer();

	bool Start(unsigned short port);
	virtual void Stop();
	CDHTThread();
	virtual ~CDHTThread();

protected:
	bool GetServerIP(std::string server, unsigned int& iip);
	void AddValueInfoOnHash(BTDHTKey &key, CDBItem &item);
	std::string GetIpString(unsigned int iip);
	void ReadNodes();
	void SaveNodes();
	virtual void Entry();
	//bool GetTextLineFromBuf(char *buf, int buflen,int& pos, std::string& sline);
private:
	bool			m_bStop;
	unsigned short	m_ListenPort;
	SockLib::CDealer*		m_pDealer;
	CDHTNode*		m_pMyNode;
	CDataBase*		m_pDataBase;
	CFrontServer*	m_pFrontServer;

	bool			m_bFindPeer;  //use dht to find peer
	bool			m_bAnnounce;	//announce ourself in dht
	bool			m_bAsServer;	//Run as a server (can store other announce)
	unsigned short	m_nLevel;		//running speed level
	char			m_sPath[512];	//savepath in utf8 format
	LOGBACK			m_logCallback;	//log callback function
	SERVICEEVENT	m_eventCallback; //event callback function
	//char			m_sPath[512];	//savepath in utf8 format
protected:

	SockLib::CMutex			m_InitialNodeMutex;
	std::list<SockLib::TInetAddr4> m_InitialNode;
};

#endif

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// FrontServer.h: interface for the CFrontServer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FRONTSERVER_H
#define _FRONTSERVER_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "TAntTask.h"
#include <SockProxyUDPServerSock.h>
#include <Mutex.h>
#include <Tools.h>
#include <list>
#include <queue>
#include <BenNode.h>

class CDHTThread;
class CBenNode;
class BTDHTKey;


class RequestCompare
{
public:
  bool operator() (  TRequestTask& lclient,  TRequestTask& rclient ) 
  {
    return (lclient.prio  < rclient.prio); //希望大的排在顶上，检查！
  }
};

typedef std::priority_queue<TRequestTask, std::vector<TRequestTask>, RequestCompare> TRequestQueue;
//typedef std::priority_queue<TRequestTask> TRequestQueue;
typedef std::list<TRequestTask> TRequestList;

class CFrontServer : public SockLib::CSockProxyUDPServerSock 
{
public:
	void SetLimit(int limit);
	//for the request by node
	void ResponseError(sockaddr_in &addr,std::string& t,int err,std::string errstr);
	//for the request by node
	void ResponseFindNode(sockaddr_in& addr,std::string& t, BencodeLib::CBenNode* arg);
	//for the request by node
	void ResponsePing(sockaddr_in& addr,std::string& t);
	//for the request by node
	void ResponseGetPeers(sockaddr_in& addr,std::string& t, BencodeLib::CBenNode* arg);
	//for the request by node
	void ResponseAnnouncePeers(sockaddr_in &addr, std::string &t, BencodeLib::CBenNode *arg);

	//for the response from node
	void OnError(sockaddr_in& addr, BencodeLib::CBenNode& ben);
	//for the response from node
	void OnResponse(sockaddr_in& addr, BencodeLib::CBenNode& ben);
	//for the response from node
	void ParsePingResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task);
	//for the response from node
	void ParseFindNodeResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task);
	//for the response from node
	void ParseGetPeersResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task);
	//for the response from node
	void ParseAnnouncePeerResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task);
	//cancel all the waiting get_peer task on hash, because the task end.
	void CancelTask(BTDHTKey& hash);
	
	void OnRequest(sockaddr_in& addr, BencodeLib::CBenNode& ben);
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnClose();
	bool DoRequest(TRequestTask& req);
	virtual void OnTimer(unsigned int id);
	bool Start(unsigned short port);
	CFrontServer(CDHTThread* parent);
	virtual ~CFrontServer();

protected:
	void DoWaitingRequest(int count);

	void SendResponsePacket(sockaddr_in& addr, unsigned short prio, std::string& content);

	bool m_bSentStartEvent;
private:

	CDHTThread* m_pParent;
	char m_MyID[20];
	int m_nRequestCounter;
	int m_nLimit;		//one second send how much packet
	unsigned int m_nTimeOutTimer; //for check the timeout of my request
	unsigned int m_nSendTimer;
	SockLib::CMutex	m_PriorityQueueMutex;
	TRequestQueue m_PriorityQueue; //the task waiting for send

	TRequestList m_PendingRequestList; //the task already send
};

#endif

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// FrontServer.cpp: implementation of the CFrontServer class.
//
//////////////////////////////////////////////////////////////////////
//Copyright liubin, multiget@gmail.com
//License: BSD

#include "../include/FrontServer.h"
#include "../include/DHTThread.h"
#include "../include/ClosestNodeStore.h"
#include "../include/DataBase.h"
#include <Dealer.h>
#include <AutoLock.h>
#include <BenNode.h>
#include <TorrentFile.h>


//#define _ONESECOND	(4)	//one second send how many packet
#define _TIMEOUT	(60)	//60s timeout wait for response

//extern void syslog( std::string info );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFrontServer::CFrontServer(CDHTThread* parent)
{
	m_pParent=parent;
	m_nRequestCounter=0;
	m_bSentStartEvent=false;
	m_nLimit=8;
}

CFrontServer::~CFrontServer()
{
	Close(); 
}

bool CFrontServer::Start(unsigned short port)
{	
	
	if(!CreateSock()) return false;

	SetDealer(m_pParent->GetDealer());

	if( !Bind( 0, htons(port) )) 
	{
		Close(); 
		OutputDebugString(L"dht bind fail");
		return false;
	}
  
	// no listen on datagram

    maskRead( true );
	
	BTDHTKey key=m_pParent->GetMyID();
	memcpy(m_MyID,(void*)(key.GetData()),20); //set the id

	//every 5s check if the request is timeout
	m_nTimeOutTimer=AddTimer(5000);
	m_nSendTimer=AddTimer(500); //every 0.5s we send out some packet.
	return true;
}

//clean our timeout request
void CFrontServer::OnTimer(unsigned int id)
{
	CSockProxyUDPServerSock::OnTimer(id);

	if(m_nTimeOutTimer==id)
	{//just check off the timeout task.

		unsigned int now=GetTickCount();

		//check and remove off the timeout request
		//find in our request for this response
		TRequestList::iterator it;


		for(it=m_PendingRequestList.begin();it!=m_PendingRequestList.end();)
		{
			if(now - (it->outtime) > _TIMEOUT*1000 )  //超时已避免积压太多包
			{
				BTDHTKey pk(it->peerkey);
				m_pParent->NoResponse(it->destnode.iip,it->destnode.iport,pk);
				m_PendingRequestList.erase(it++);
				continue;
			}

			it++;
		}

	}
	else if(m_nSendTimer==id)
	{

		//ok, we send some packet
		static unsigned int last=0;
		unsigned int now=GetTickCount();
		if(last==0)
		{
			last=now;
			DoWaitingRequest(m_nLimit);
		}
		else
		{
			if(now < last)
			{
				last=now;
				DoWaitingRequest(m_nLimit);
			}
			else
			{
				unsigned int gap=now-last;
				last=now;
				DoWaitingRequest(int(m_nLimit* (float(gap)/1000.0)));
			}
		}
	}
}

bool CFrontServer::DoRequest(TRequestTask &req)
{
	SockLib::CAutoLock al(m_PriorityQueueMutex);
	m_PriorityQueue.push(req); //放进排队里
	return true;
}


void CFrontServer::OnRead()
{
	CSockProxyUDPServerSock::OnRead();

	//yes can read the message now 
    char  buf[ 8*1024 ];

    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );

#ifdef WIN32
    int len = sizeof( addr );
#else
    socklen_t len = ( socklen_t ) sizeof( addr );
#endif

    int ret = recvfrom( m_hSocket, buf, 8*1024, 0, ( struct sockaddr* ) & addr, &len );

	
	if(ret<=0)
	{
#ifdef WIN32
		switch(ret)
		{
		case WSAENETDOWN:	//net down
			break;
		case WSAESHUTDOWN:	//shut down
			break;
		default:
			break;
		}
#endif
		Sleep(20);
		return;
	}

	//CTorrentFile tf;
	//if(0!=tf.ReadBuf(buf,ret)) 
	//{
	//	return; //error packet
	//}

	//CBenNode& ben=tf.GetRootNode();
	std::string tf;
	tf.append(buf, ret);
	BencodeLib::CBenNode ben;
	if(0!=ben.FromStream(tf)) return; //error benformat.


	BencodeLib::CBenNode* ty=ben.FindKeyValue("y");
	if(ty==NULL||ty->GetType()!=BencodeLib::beString) 
	{
		//no 'y' key ,ignore
		return;
	}
	
	std::string tystr;
	ty->GetStringValue(tystr);
	
	if(tystr==std::string("r"))
	{
		OnResponse(addr,ben);
	}
	else if(tystr==std::string("q"))
	{
		OnRequest(addr,ben);
	}
	else if(tystr==std::string("e"))
	{
		OnError(addr,ben);
	}
	else
	{
		//unkonwn packet type,just ignore;
	}

}

void CFrontServer::OnClose()
{
 
}

void CFrontServer::OnWrite()
{
	CSockProxyUDPServerSock::OnWrite();
}

void CFrontServer::OnRequest(sockaddr_in& addr, BencodeLib::CBenNode &ben)
{

	//it's a request from node ,just do the command
	BencodeLib::CBenNode* tt=ben.FindKeyValue("t");
	if(tt==NULL||tt->GetType()!=BencodeLib::beString) 
	{
		return;
	}

	std::string tstr;
	tt->GetStringValue(tstr);


	//query type string
	BencodeLib::CBenNode* tq=ben.FindKeyValue("q");
	if(tq==NULL||tq->GetType()!=BencodeLib::beString) 
	{//query type string
		ResponseError(addr,tstr,203,"Protocol Error: No 'q' argument or wrong type.");
		return;
	}

	std::string qstr;
	tq->GetStringValue(qstr);

	if(qstr==std::string("ping"))
	{
		ResponsePing(addr,tstr);
	}
	else if(qstr==std::string("find_node"))
	{
		//get the peer key and compare to our kbucket,return 8 closest node to him
		BencodeLib::CBenNode* arg=ben.FindKeyValue("a");
		if(arg==NULL||arg->GetType()!=BencodeLib::beDict) {
			ResponseError(addr,tstr,201,"No argument dictionary or wrong type.");
			return;
		}

		ResponseFindNode(addr,tstr,arg);
	}
	else if(qstr==std::string("get_peers"))
	{
		BencodeLib::CBenNode* arg=ben.FindKeyValue("a");
		if(arg==NULL||arg->GetType()!=BencodeLib::beDict) {
			ResponseError(addr,tstr,201,"No argument dictionary or wrong type.");
			return;
		}

		//check if we have the data of peers, if yes,return the peers value
		//if not ,return the closest node to him,anyway, a token should give back for
		//future announce
		//printf("get_peers request\n");
		ResponseGetPeers(addr,tstr,arg);
	}
	else if(qstr==std::string("announce_peer"))
	{
		BencodeLib::CBenNode* arg=ben.FindKeyValue("a");
		if(arg==NULL||arg->GetType()!=BencodeLib::beDict) {
			ResponseError(addr,tstr,201,"No argument dictionary or wrong type.");
			return;
		}

		ResponseAnnouncePeers(addr,tstr,arg);

	}
	else
	{
		//give a error message
		ResponseError(addr,tstr,204,"Method Unknown");
		return; //don't know the query type
	}

	//update this node for the last seen
	BencodeLib::CBenNode* arg=ben.FindKeyValue("a");
	if(arg==NULL||arg->GetType()!=BencodeLib::beDict) return;

	std::string idstr;
	BencodeLib::CBenNode* id=arg->FindKeyValue("id");
	if(id==NULL||id->GetType()!=BencodeLib::beString) return;

	id->GetStringValue(idstr);
	if(idstr.size()!=20) return;

	BTDHTKey idkey(idstr.data());
	SockLib::TInetAddr4 iaddr;
#ifdef WIN32
	iaddr.iip=(addr.sin_addr.S_un.S_addr);
#else
	iaddr.iip=(addr.sin_addr.s_addr);
#endif
	iaddr.iport=(addr.sin_port);

	m_pParent->UpdateForRequest(iaddr,idkey);

}

void CFrontServer::OnResponse(sockaddr_in& addr, BencodeLib::CBenNode &ben)
{


	BencodeLib::CBenNode* tt=ben.FindKeyValue("t");
	if(tt==NULL||tt->GetType()!=BencodeLib::beString) 
	{
		return;
	}

	std::string tstr;
	tt->GetStringValue(tstr);

	if(tstr.empty()) return;

	char tc=tstr[0];

	SockLib::TInetAddr4 saddr;
#ifdef WIN32
	saddr.iip=(addr.sin_addr.S_un.S_addr);
#else
	saddr.iip=(addr.sin_addr.s_addr);
#endif
	saddr.iport=(addr.sin_port);

	//find in our request for this response
	
	bool found=false;
	
	TRequestList::iterator it;
	for(it=m_PendingRequestList.begin();it!=m_PendingRequestList.end();it++)
	{
		if(it->destnode==saddr && it->sid==tc)
		{
			found=true;
			TRequestTask tcopy=(*it);  //copy is checked ok
			//found the record and parse the packet
			switch(it->tasktype)
			{
			case TSK_PING:
				ParsePingResponse(addr,ben,&tcopy);
				break;
			case TSK_FIND_NODE:
				ParseFindNodeResponse(addr,ben,&tcopy);
				break;
			case TSK_GET_PEERS:
				ParseGetPeersResponse(addr,ben,&tcopy);
				break;
			case TSK_ANNOUNCE_PEER:
				ParseAnnouncePeerResponse(addr,ben,&tcopy);
				break;
			default:
				break;
			}

			m_PendingRequestList.erase(it);
			break;
		}
		else if(it->destnode.iip==saddr.iip && it->sid==tc) //some node return from diff port!!
		{	
			//make compatiable!
			found=true;

			TRequestTask tcopy=(*it);  //copy is checked ok
			//found the record and parse the packet
			switch(it->tasktype)
			{
			case TSK_PING:
				ParsePingResponse(addr,ben,&tcopy);
				break;
			case TSK_FIND_NODE:
				ParseFindNodeResponse(addr,ben,&tcopy);
				break;
			case TSK_GET_PEERS:
				ParseGetPeersResponse(addr,ben,&tcopy);
				break;
			case TSK_ANNOUNCE_PEER:
				ParseAnnouncePeerResponse(addr,ben,&tcopy);
				break;
			default:
				break;
			}

			m_PendingRequestList.erase(it);
			break;			
			
		}

	}

	if(!found) {
		//printf("can't find origin request for response from %s:%d\n",saddr.ip.c_str(),saddr.port);
		return; //invalid data don't send event.
	}
		
}

void CFrontServer::OnError(sockaddr_in& addr, BencodeLib::CBenNode &ben)
{

	BencodeLib::CBenNode* tt=ben.FindKeyValue("t");
	if(tt==NULL||tt->GetType()!=BencodeLib::beString) 
	{
		return ; //wrong packet
	}

	std::string tstr;
	tt->GetStringValue(tstr);
	if(tstr.empty()) return;

	char tc=tstr[0];
	//error message, we find in our request to find out which request should be removed
	SockLib::TInetAddr4 saddr;
#ifdef WIN32
	saddr.iip=(addr.sin_addr.S_un.S_addr);
#else
	saddr.iip=(addr.sin_addr.s_addr);
#endif

	saddr.iport=(addr.sin_port);

	//find in our request for this response
	TRequestList::iterator it;
	for(it=m_PendingRequestList.begin();it!=m_PendingRequestList.end();it++)
	{
		if(it->destnode==saddr && it->sid==tc)
		{
			//give error response to upper
			m_PendingRequestList.erase(it);
			return;
		}
		else if(it->destnode.iip==saddr.iip && it->sid==tc)
		{
			m_PendingRequestList.erase(it);
			return;			
		}
	}

}

//just response the request an error
void CFrontServer::ResponseError(sockaddr_in &addr, std::string &t, int err, std::string errstr)
{

	std::string content;
	BencodeLib::CBenNode ben;
	ben.OpenDictionary();
	ben.AddValue("t"); //transaction ID key
	ben.AddValue(t); //default ID is 0;
		
	ben.AddValue("y"); //type
	ben.AddValue("e"); //the type is response
		
	ben.AddValue("e");	//arguments key

	ben.OpenList();
	ben.AddValue(llong(err));
	ben.AddValue(errstr);
	ben.CloseList();

	ben.CloseDictionary();
		
	ben.ToStream(content);

	SendResponsePacket(addr,_ERROR_RES_PRIO,content);

}
void CFrontServer::ResponseFindNode(sockaddr_in& addr,std::string& t, BencodeLib::CBenNode* arg)
{
	//find the closest K node
	BencodeLib::CBenNode* at=arg->GetKeyValue("target");
	if(at==NULL||at->GetType()!=BencodeLib::beString)
	{
		ResponseError(addr,t,203,"No target argument");
		return;
	}

	std::string tarstr;
	at->GetStringValue(tarstr);
	if(tarstr.size()!=20) 
	{
		ResponseError(addr,t,203,"target length!=20");
		return;
	}

	BTDHTKey target(tarstr.data());

	SockLib::TInetAddr4 entryaddr;

	if(m_pParent->GetEntryAddr(target,entryaddr))
	{

		std::string content;

		std::string compact=tarstr;

		std::string ipport;

		ipport.append((const char*)(&entryaddr.iip),4);
		ipport.append((const char*)(&entryaddr.iport),2);

		tarstr+=ipport;
		
		//the node in my bucket,just return it
		BencodeLib::CBenNode ben;
		ben.OpenDictionary();
		ben.AddValue("t"); //transaction ID key
		ben.AddValue(t); //default ID is 0;
		
		ben.AddValue("y"); //type
		ben.AddValue("r"); //the type is response
		
		ben.AddValue("r");	//arguments key

		ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
		ben.AddValue("id");
		ben.AddValue(&m_MyID[0],20); //here is my nodeid
		
		ben.AddValue("nodes");
		ben.AddValue(compact);
		
		ben.CloseDictionary();
		ben.CloseDictionary();
		
		ben.ToStream(content);
		
		//and then send the packet
		SendResponsePacket(addr, _FIND_NODE_RES_PRIO, content);
	}
	else
	{
		//the node not in my bucket, return the closest 8 nodes
		CClosestNodeStore store(target,8);
		m_pParent->GetClosestGoodNode(store);

		if(store.GetCount()>0)
		{
			std::string content;
			//make the nodes string
			std::string nodesstr;

			CClosestNodeStore::Itr it;
			for(it=store.begin();it!=store.end();it++)
			{
				BTDHTKey key=it->second.GetKey();
				nodesstr.append((const char*)(key.GetData()),20);
				SockLib::TInetAddr4 naddr=it->second.GetAddress();

				std::string ipport;
				ipport.append((const char*)(&naddr.iip),4);
				ipport.append((const char*)(&naddr.iport),2);
				nodesstr+=ipport;

			}

			BencodeLib::CBenNode ben;
			ben.OpenDictionary();
			ben.AddValue("t"); //transaction ID key
			ben.AddValue(t); //default ID is 0;
			
			ben.AddValue("y"); //type
			ben.AddValue("r"); //the type is response
			
			ben.AddValue("r");	//arguments key
			ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
			ben.AddValue("id");
			ben.AddValue(&m_MyID[0],20); //here is my nodeid
			
			ben.AddValue("nodes");
			ben.AddValue(nodesstr);
			
			ben.CloseDictionary();
			ben.CloseDictionary();
			
			ben.ToStream(content);

			SendResponsePacket(addr, 3, content);

		}
		else
		{
			ResponseError(addr,t,202,"Sorry but no closer node avialble");
		}
	}

}

void CFrontServer::ResponseGetPeers(sockaddr_in& addr,std::string& t, BencodeLib::CBenNode* arg)
{

	BencodeLib::CBenNode* ai=arg->GetKeyValue("info_hash");
	if(ai==NULL||ai->GetType()!=BencodeLib::beString) {
		ResponseError(addr,t,203,"No info_hash key or wrong type");
		return;
	}

	
	//TODO: check if we have closer node to give ,if yes, return the closer nodes;
	//if not try to find value in db.
	//if no value and no nodes to give ,return empty value list.
	
	std::string infostr;
	ai->GetStringValue(infostr);

	if(infostr.size()!=20) {
		ResponseError(addr,t,203,"Protocol Error: Wrong info_hash key length");
		return;
	}


	BTDHTKey token=m_pParent->GetDataBase()->GenToken(addr);

	//应该优先查找最近的8个节点，如果我们自己在其中，那么我们再找数据库中的值
	//如果数据库中没有数据，返回最近的节点和token
	//如果数据库中有数据，返回数据
	BTDHTKey infokey(infostr.data());
	CClosestNodeStore store(infokey,8);
	m_pParent->GetClosestGoodNode(store);

	BTDHTKey myidkey(m_MyID);
	
	if(store.IsNearer(myidkey))
	{
		//我们自己还更近点，那么，应该返回数据库中的值，如果没有值，才返回这些节点

		DBItemList dbl;
		//find our data for this infohash
		m_pParent->GetDataBase()->Sample(infokey,dbl,50); //max=50

		if(dbl.size()>0)
		{//got the value
			
			
			BencodeLib::CBenNode ben;
			ben.OpenDictionary();
			ben.AddValue("t"); //transaction ID key
			ben.AddValue(t); //default ID is 0;
			
			ben.AddValue("y"); //type
			ben.AddValue("r"); //the type is response
			
			ben.AddValue("r");	//arguments key
			ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
			
			ben.AddValue("token");	//token在r字典里！！
			ben.AddValue((char*)(token.GetData()),20);		
			
			ben.AddValue("id");
			ben.AddValue(&m_MyID[0],20); //here is my nodeid
			
			ben.AddValue("values");  //values is a list
			ben.OpenList();
			
			//add all the peer info as list items
			DBItemList::iterator it;
			for(it=dbl.begin();it!=dbl.end();it++)
			{
				ben.AddValue((char*)(it->GetData()),6);
			}
			
			ben.CloseList();
			
			ben.CloseDictionary();
			ben.CloseDictionary();
			std::string content;			
			ben.ToStream(content);

			SendResponsePacket(addr, _GET_PEER_RES_PRIO, content);
			return;
		}

	}
	
	//返回节点列表,一种情况是我们不在最近表中，还有是我们没有数据

	if(store.GetCount()>0)
	{
		//make the nodes string
		std::string nodesstr;
		
		CClosestNodeStore::Itr it;
		for(it=store.begin();it!=store.end();it++)
		{
			BTDHTKey key=it->second.GetKey();
			nodesstr.append((const char*)(key.GetData()),20);
			SockLib::TInetAddr4 naddr=it->second.GetAddress();
			
			std::string ipport;

			ipport.append((const char*)(&naddr.iip),4);
			ipport.append((const char*)(&naddr.iport),2);
			nodesstr+=ipport;

		}
		
		BencodeLib::CBenNode ben;
		ben.OpenDictionary();
		ben.AddValue("t"); //transaction ID key
		ben.AddValue(t); //default ID is 0;
		
		ben.AddValue("y"); //type
		ben.AddValue("r"); //the type is response
		
		ben.AddValue("r");	//arguments key
		ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
		
		ben.AddValue("token");	//token
		ben.AddValue((char*)(token.GetData()),20);
		
		ben.AddValue("id");
		ben.AddValue(&m_MyID[0],20); //here is my nodeid
		
		ben.AddValue("nodes");
		ben.AddValue(nodesstr);
		
		ben.CloseDictionary();
		ben.CloseDictionary();
		std::string content;		
		ben.ToStream(content);

		SendResponsePacket(addr, _GET_PEER_RES_PRIO, content);

	}
	else
	{
		//也可以返回values和nodes都为空的包，暂时返回错误，应该是我们自己还没有引导起来
		//否则不可能都为空
		//no result, return an error
		ResponseError(addr,t,202,"Sorry but no peer value or closer node avialble");

	}


}

void CFrontServer::ResponsePing(sockaddr_in& addr,std::string& t)
{
	BencodeLib::CBenNode ben;
	ben.OpenDictionary();
	ben.AddValue("t"); //transaction ID key
	ben.AddValue(t); 

	ben.AddValue("y"); //type
	ben.AddValue("r"); //the type is response

	//when response type, also have a dict 'r'
	ben.AddValue("r");	//arguments key
	ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
	ben.AddValue("id");
	ben.AddValue(&m_MyID[0],20); //here is my nodeid
	ben.CloseDictionary();
	ben.CloseDictionary();
	//and then send the packet

	std::string content;
	ben.ToStream(content);

	SendResponsePacket(addr, _PING_RES_PRIO, content);

}

void CFrontServer::ResponseAnnouncePeers(sockaddr_in &addr, std::string &t, BencodeLib::CBenNode *arg)
{
//arguments: {"id" : "<querying nodes id>", "info_hash" : "<20-byte infohash of target torrent>", "port" : <port number>, "token" : "<opaque token>"} 
//response: {"id" : "<queried nodes id>"}

	BencodeLib::CBenNode* ai=arg->GetKeyValue("info_hash");
	if(ai==NULL||ai->GetType()!=BencodeLib::beString) {
		ResponseError(addr,t,203,"No info_hash key or wrong type");
		return;
	}

	std::string shash;
	ai->GetStringValue(shash);

	if(shash.size()!=20) {
		ResponseError(addr,t,203,"Wrong info_hash key length");
		return;
	}

	BencodeLib::CBenNode* ap=arg->GetKeyValue("port");
	if(ap==NULL||ap->GetType()!=BencodeLib::beInt) {
		ResponseError(addr,t,203,"No port key or wrong type");
		return;
	}

	int port=ap->GetIntValue();

	BencodeLib::CBenNode* at=arg->GetKeyValue("token");
	if(at==NULL||at->GetType()!=BencodeLib::beString) {
		ResponseError(addr,t,203,"No token or wrong type");
		return;
	}

	std::string stoken;
	at->GetStringValue(stoken);

	if(stoken.size()!=20)	{  //our token is 20 bytes len.
		ResponseError(addr,t,201,"Invalid token");
		return;
	}
	
	SockLib::TInetAddr4 iaddr;
#ifdef WIN32
	iaddr.iip=(addr.sin_addr.S_un.S_addr);
#else
	iaddr.iip=(addr.sin_addr.s_addr);
#endif

	iaddr.iport=(addr.sin_port);
	
	int nret=m_pParent->DoPeerAnnounceTask(stoken,shash,iaddr,port);
	if(nret<0)
	{ //fail to store data, response error.
		switch(nret)
		{
			case -1:
				ResponseError(addr,t,204,"Service unavialable");
				break;
			case -2:
				ResponseError(addr,t,204,"token length error.");
				break;
			case -3:
				ResponseError(addr,t,204,"token check error.");
				break;
			case -4:
				ResponseError(addr,t,204,"Wrong ip or port");
				break;
			case -5:
				ResponseError(addr,t,204,"fail to store");
				break;
			default:
				break;
		}		
		return;		
	}


	//give good response
	char buf[512];
	int pos=0;

	BencodeLib::CBenNode ben;
	ben.OpenDictionary();
	ben.AddValue("t",1); //transaction ID key
	ben.AddValue((char*)(t.data()),t.size()); //default ID is 0;
	
	ben.AddValue("y",1); //type
	ben.AddValue("r",1); //the type is response
	
	ben.AddValue("r",1);	//arguments key
	ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
	ben.AddValue("id",2);
	ben.AddValue(&m_MyID[0],20); //here is my nodeid
	
	ben.CloseDictionary();
	ben.CloseDictionary();

	std::string content;
	content.append(buf,pos);
	SendResponsePacket(addr, _ANNOUNCE_RES_PRIO, content);
}


void CFrontServer::ParsePingResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task)
{

	BencodeLib::CBenNode* tr=ben.FindKeyValue("r"); // value is dict
	
	if(tr==NULL||tr->GetType()!=BencodeLib::beDict)
	{
		BTDHTKey key(task->peerkey);
		m_pParent->ErrorPingResponse(task->destnode.iip,task->destnode.iport,key);
		return;
	}
	
	BencodeLib::CBenNode* id=tr->FindKeyValue("id");
	if(id==NULL||id->GetType()!=BencodeLib::beString)
	{
		BTDHTKey key(task->peerkey);
		m_pParent->ErrorPingResponse(task->destnode.iip,task->destnode.iport,key);
		return;
	}
	
	std::string idstr;
	id->GetStringValue(idstr);
	if(idstr.size()!=20)
	{
		BTDHTKey key(task->peerkey);
		m_pParent->ErrorPingResponse(task->destnode.iip,task->destnode.iport,key);
		return;
	}
	else
	{
		BTDHTKey key((const char*)(idstr.data())); //return the key of node
		m_pParent->GoodPingResponse(task->destnode.iip,task->destnode.iport,key);
		return;
	}

	return;
}

void CFrontServer::ParseFindNodeResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task)
{

	BencodeLib::CBenNode* tr=ben.FindKeyValue("r"); // value is dict
	
	if(tr==NULL||tr->GetType()!=BencodeLib::beDict)
	{
		return;
	}
	
	
	BencodeLib::CBenNode* id=tr->FindKeyValue("id");
	if(id==NULL||id->GetType()!=BencodeLib::beString)
	{
		return;
	}

	std::string idstr;
	id->GetStringValue(idstr);
	if(idstr.size()!=20)
	{
		return;
	}

	BTDHTKey peerid(idstr.data());
	
	BencodeLib::CBenNode* nodes=tr->FindKeyValue("nodes");
	if(nodes==NULL||nodes->GetType()!=BencodeLib::beString)
	{
		m_pParent->ErrorFindNodeResponse(task->destnode.iip,task->destnode.iport,peerid);
		return;
	}
	
	std::string nodesstr;
	nodes->GetStringValue(nodesstr);
	//Contact information for nodes is encoded as a 26-byte string. Also known as "Compact node info" the 20-byte Node ID in network byte order has the compact IP-address/port info concatenated to the end.
	if(idstr.size()!=20 || nodesstr.size()%26!=0)
	{
		m_pParent->ErrorFindNodeResponse(task->destnode.iip,task->destnode.iport,peerid);
		return;
	}
	else
	{
		BTDHTKey target(task->targetkey);
		m_pParent->GoodFindNodeResponse(task->destnode.iip,task->destnode.iport,peerid,nodesstr,target);
		return;
	}
	
	return;
}


//get_peers also do announce job
void CFrontServer::ParseGetPeersResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task)
{

	BencodeLib::CBenNode* tr=ben.FindKeyValue("r"); // value is dict
	
	if(tr==NULL||tr->GetType()!=BencodeLib::beDict)
	{
		return;
	}
	
	
	BencodeLib::CBenNode* id=tr->FindKeyValue("id");
	if(id==NULL||id->GetType()!=BencodeLib::beString)
	{
		return;
	}
	
	std::string idstr;
	id->GetStringValue(idstr);

	if(idstr.size()!=20)
	{
		return;
	}

	BTDHTKey peerid(idstr.data());

	//one of them must exists
	BencodeLib::CBenNode* nodes=tr->FindKeyValue("nodes");
	BencodeLib::CBenNode* value=tr->FindKeyValue("values");
	if(nodes==NULL && value==NULL)
	{
		BencodeLib::CBenNode* token=tr->FindKeyValue("token");
		if(token!=NULL && token->GetType()==BencodeLib::beString)
		{
			std::string tokenstr;
			token->GetStringValue(tokenstr);
			
			std::string infohash;
			infohash.append(task->targetkey,20); //the get_peer target is infohash
			m_pParent->DoAnnounceTask(tokenstr, infohash, task->destnode.iip, task->destnode.iport);
		}

		return;
	}
	
	if(nodes!=NULL && value!=NULL)
	{
		m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
		return;
	}
	
	if(nodes!=NULL && nodes->GetType()==BencodeLib::beString)
	{//nodes valid
		
		std::string nodesstr;
		nodes->GetStringValue(nodesstr);
		if(nodesstr.size()%26 !=0||nodesstr.empty())
		{
			m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
		}
		else
		{//good, we got the other nodes info
			

			//BTDHTKey key((const char*)(idstr.data())); //return the key of node
			BTDHTKey target(task->targetkey);

			//compare the returned node distance from hash.
			//if this node is nearer than one of the returned nodes,
			//then we can announce to this node, because we pick 8 nodes to announce to.
			//and compare thisnodedist to the nodes it returned
			//is thisnodedist < some of the nodes distance.
			
			BTDHTKey thisnodedist = target- peerid;			
			bool announcethis=false;
			for(int n=0;n<nodesstr.size()/26;n++)
			{
				std::string snodekey=nodesstr.substr(n*26,20);
				BTDHTKey nodekey(snodekey.data());
				BTDHTKey dist=target-nodekey;
				if(thisnodedist < dist)
				{
					announcethis=true;
					break;
				}					
			}

			if(announcethis)
			{
				//m_pParent->LogMsg(L"do announce because this node is nearest",MSG_INFO);

				BencodeLib::CBenNode* token=tr->FindKeyValue("token"); 
				if(token!=NULL && token->GetType()==BencodeLib::beString)
				{
					std::string tokenstr;
					token->GetStringValue(tokenstr);
					
					std::string infohash;
					infohash.append(task->targetkey,20); //the get_peer target is infohash
					
					m_pParent->DoAnnounceTask(tokenstr, infohash, task->destnode.iip, task->destnode.iport);
				}

			}//if(announcethis)
			
			m_pParent->GoodGetPeerResponse(task->destnode.iip,task->destnode.iport,peerid,target,false,nodesstr);
			
		}

	}
	else if(value!=NULL && value->GetType()==BencodeLib::beList)
	{//value valid
		
		if(value->GetNumberOfList()==0)
		{
			m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
			return;
		}

		int nol=value->GetNumberOfList();

		std::string allvalue; //all the peers compact

		for(int i=0;i<nol;i++)
		{//parse every list of value
			BencodeLib::CBenNode* valuelist=value->GetListMember(i);

			if(valuelist==NULL||valuelist->GetType()!=BencodeLib::beString)
			{
				m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
				return;
			}

			std::string valuestr;
			valuelist->GetStringValue(valuestr);
			if(valuestr.size() !=6) //26??
			{
				m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
				return;
			}

			allvalue.append(valuestr);

		}

		BTDHTKey key((const char*)(idstr.data()));
		BTDHTKey target(task->targetkey);
		m_pParent->GoodGetPeerResponse(task->destnode.iip,task->destnode.iport,key,target,true,allvalue);

		//when we got values, must try announce.
		//check if have token in the dict ,if have ,we announce ourself
		//m_pParent->LogMsg(L"do announce because it give values",MSG_INFO);

		BencodeLib::CBenNode* token=tr->FindKeyValue("token");
		if(token!=NULL && token->GetType()==BencodeLib::beString)
		{
			std::string tokenstr;
			token->GetStringValue(tokenstr);
			
			std::string infohash;
			infohash.append(task->targetkey,20); //the get_peer target is infohash
			
			m_pParent->DoAnnounceTask(tokenstr, infohash, task->destnode.iip, task->destnode.iport);
		}
	
		return;
	}
	else
	{
		m_pParent->ErrorGetPeersResponse(task->destnode.iip,task->destnode.iport,peerid);
	}

	return;
}

void CFrontServer::ParseAnnouncePeerResponse(sockaddr_in &addr, BencodeLib::CBenNode& ben, TRequestTask* task)
{
	
	BencodeLib::CBenNode* tr=ben.FindKeyValue("r"); // value is dict

	if(tr==NULL||tr->GetType()!=BencodeLib::beDict)
	{
		m_pParent->ErrorAnnouncePeersResponse(task->destnode.iip,task->destnode.iport);
		return;
	}
	
	
	BencodeLib::CBenNode* id=tr->FindKeyValue("id");
	if(id==NULL||id->GetType()!=BencodeLib::beString)
	{
		return;
	}
	
	std::string idstr;
	id->GetStringValue(idstr);

	if(idstr.size()!=20)
	{
		return;
	}
	else
	{
		BTDHTKey peerid(idstr.data());
		m_pParent->GoodAnnouncePeersResponse(task->destnode.iip,task->destnode.iport, peerid);
		return;
	}

}

void CFrontServer::DoWaitingRequest(int count)
{

	SockLib::CAutoLock al(m_PriorityQueueMutex);

	for(int pc=0; pc<count && (!m_PriorityQueue.empty()); pc++)  
	{

		//取出队头一个任务
		TRequestTask req=m_PriorityQueue.top();
		//在队伍中删除这个任务
		m_PriorityQueue.pop();

		if(req.response==0)
		{//请求包
			char sidarray[10]; //'0'-'9'
			//选择一个包序号，避免对同一个地址发送多个包中有相同的包号
			//当对方回应时我们才能根据这个包号找到原始请求

			for(int i=0;i<10;i++) sidarray[i]='0'+i; //重置所有ID可用

			TRequestList::iterator it;
			for(it=m_PendingRequestList.begin();it!=m_PendingRequestList.end();it++)
			{
				if(it->destnode.iip==req.destnode.iip)
				{
					//mark off the table
					sidarray[it->sid - '0']=0;  //mark this id used

				}
			}

			//then find in the table left
			//req.sid='0';

			req.sid=0;
			for(int j=0;j<10;j++)
			{
				if(sidarray[j])
				{
					req.sid=sidarray[j]; //need check the list to get sid
					break;
				}
			}

			if(req.sid==0) {
				continue;  //too many pending request to one dest, give up
			}


			std::string content;

			switch(req.tasktype)
			{
			case TSK_PING:
				{

					BencodeLib::CBenNode ben;
					ben.OpenDictionary();
					ben.AddValue("t"); //transaction ID key
					ben.AddValue((char*)(&req.sid),1); //default ID is 0;
					ben.AddValue("y"); //type
					ben.AddValue("q"); //the type is query

					ben.AddValue("q");
					ben.AddValue("ping");  //the query type

					ben.AddValue("a");	//arguments key
					ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789"}
					ben.AddValue("id");
					ben.AddValue(&m_MyID[0],20); //here is my nodeid
					ben.CloseDictionary();
					ben.CloseDictionary();

					ben.ToStream(content);

				}
				break;
			case TSK_FIND_NODE:
				{

					BencodeLib::CBenNode ben;
					ben.OpenDictionary();
					ben.AddValue("t"); //transaction ID key
					ben.AddValue((char*)(&req.sid),1); //default ID is 0;
					ben.AddValue("y"); //type
					ben.AddValue("q"); //the type is query
					//when query type, also have a key 'q'
					ben.AddValue("q");
					ben.AddValue("find_node");  //the query type
					ben.AddValue("a"); //arguments key
					ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789","target":"xxxxxxxxxxx"}
					ben.AddValue("id");
					ben.AddValue(m_MyID,20); //here is my nodeid
					ben.AddValue("target");
					ben.AddValue(req.targetkey,20);
					ben.CloseDictionary();
					ben.CloseDictionary();

					ben.ToStream(content);

				}
				break;
			case TSK_GET_PEERS:
				{

					BencodeLib::CBenNode ben;
					ben.OpenDictionary();
					ben.AddValue("t"); //transaction ID key
					ben.AddValue((char*)(&req.sid),1); //ID is '0'-'9';
					ben.AddValue("y"); //type
					ben.AddValue("q"); //the type is query
					//when query type, also have a key 'q'
					ben.AddValue("q");
					ben.AddValue("get_peers");  //the query type
					ben.AddValue("a"); //arguments key
					ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789","target":"xxxxxxxxxxx"}
					ben.AddValue("id");
					ben.AddValue(m_MyID,20); //here is my nodeid
					ben.AddValue("info_hash");
					ben.AddValue(req.targetkey,20);
					ben.CloseDictionary();
					ben.CloseDictionary();

					ben.ToStream(content);

				}
				break;
			case TSK_ANNOUNCE_PEER:  //announce to the DHT net that we are downloading some torrent
				{

					BencodeLib::CBenNode ben;
					ben.OpenDictionary();
					ben.AddValue("t"); //transaction ID key
					ben.AddValue((char*)(&req.sid),1); //default ID is 0;
					ben.AddValue("y"); //type
					ben.AddValue("q"); //the type is query
					//when query type, also have a key 'q'
					ben.AddValue("q");
					ben.AddValue("announce_peer"); //maybe announce_peer is ok
					ben.AddValue("a"); //arguments key
					ben.OpenDictionary();  //here goes 'a' dictionary "a":{"id":"abcdefghij0123456789","target":"xxxxxxxxxxx"}
					ben.AddValue("id");
					ben.AddValue(m_MyID,20); //here is my nodeid
					ben.AddValue("info_hash");
					ben.AddValue(req.targetkey,20);
					ben.AddValue("port");
					ben.AddValue(llong(req.lport));
					ben.AddValue("token");
					ben.AddValue((char*)(req.token.data()),req.token.size());
					ben.CloseDictionary();
					ben.CloseDictionary();

					ben.ToStream(content);

				}
				break;
			default:
				continue;

			}

			struct sockaddr_in addr;

			addr.sin_family = AF_INET;
	#ifdef WIN32
			addr.sin_addr.S_un.S_addr = (req.destnode.iip);
	#else
			addr.sin_addr.s_addr = (req.destnode.iip);
	#endif
			addr.sin_port = ( req.destnode.iport );

			sendto( m_hSocket, content.data(), content.size(), 0, ( const struct sockaddr* ) & addr, sizeof( addr ) );

			req.outtime=GetTickCount();	

			m_PendingRequestList.push_back(req); //save this packet to pending list
		}
		else
		{//反馈包，直接发送无须保留到发送表里等待反馈
			struct sockaddr_in addr;

			addr.sin_family = AF_INET;
	#ifdef WIN32
			addr.sin_addr.S_un.S_addr = (req.destnode.iip);
	#else
			addr.sin_addr.s_addr = (req.destnode.iip);
	#endif
			addr.sin_port = ( req.destnode.iport );

			sendto( m_hSocket, req.token.data(), req.token.size(), 0, ( const struct sockaddr* ) & addr, sizeof( addr ) );
		}

	}

	return ;
}



//other thread call this function should lock first
void CFrontServer::CancelTask(BTDHTKey& hash)
{

	//cancle get_peers and announce task on hash

	
	SockLib::CAutoLock al(m_PriorityQueueMutex);
	std::list<TRequestTask> swap;
	
	while(!m_PriorityQueue.empty())
	{
		TRequestTask req=m_PriorityQueue.top();
		m_PriorityQueue.pop();
		if(req.tasktype==TSK_GET_PEERS||req.tasktype==TSK_ANNOUNCE_PEER) 
		{
			BTDHTKey hashkey(req.targetkey);
			if(hashkey==hash)
			{
				continue;
			}
			else
			{
				swap.push_back(req);
			}
		}
		else
		{
			swap.push_back(req);
		}
	}
	
	//write back left to queue
	while(!swap.empty())
	{
		TRequestTask req=swap.front();
		swap.pop_front();
		m_PriorityQueue.push(req);
	}
	

}

//发送反馈包，反馈包进入排队的扩展
void CFrontServer::SendResponsePacket(sockaddr_in& addr, unsigned short prio, std::string& content)
{
	//改为排队去

	TRequestTask res;

	res.response=1;
	res.prio=prio;

#ifdef WIN32
	res.destnode.iip=addr.sin_addr.S_un.S_addr;
#else
	res.destnode.iip=addr.sin_addr.s_addr;
#endif

	res.destnode.iport=addr.sin_port;

	res.token=content;

	SockLib::CAutoLock al(m_PriorityQueueMutex);
	m_PriorityQueue.push(res);
}

void CFrontServer::SetLimit(int limit)
{
	m_nLimit=limit;
}

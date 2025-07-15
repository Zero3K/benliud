/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

// DHTThread.cpp: implementation of the CDHTThread class.
//
//////////////////////////////////////////////////////////////////////



#include "../include/DHTThread.h"
#include <Dealer.h>
#include "../include/DHTNode.h"
#include "../include/DataBase.h"
#include "../include/FrontServer.h"
#include "../include/DBItem.h"
#include <AutoLock.h>

#include <iostream>
#include <fstream>

extern wchar_t gSavePath[MAX_PATH];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void syslog(std::string);

CDHTThread::CDHTThread()
{
	m_bStop=false;

	m_pDealer=NULL;
	m_pFrontServer=NULL;
	m_pMyNode=NULL;
	m_pDataBase=NULL;
	m_nLevel=5;
	m_logCallback=NULL;
	m_eventCallback=NULL;
	memset(m_sPath, 0, sizeof(m_sPath));
}

CDHTThread::~CDHTThread()
{
	if(m_pDealer!=NULL)
		delete m_pDealer;

	if(m_pFrontServer!=NULL)
		delete m_pFrontServer;

	if(m_pMyNode!=NULL)
		delete m_pMyNode;

	if(m_pDataBase!=NULL)
		delete m_pDataBase;

}

void CDHTThread::Entry()
{
#if defined( WIN32)||defined(WINCE)
    WSADATA wsadata;
    WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
#endif

	//add fix bootup nodes
	AddFixNodes();

	m_pFrontServer=new CFrontServer(this);

	switch(m_nLevel)
	{
	case 1:
		m_pFrontServer->SetLimit(2);
		break;
	case 2:
		m_pFrontServer->SetLimit(4);
		break;
	case 3:
		m_pFrontServer->SetLimit(6);
		break;
	case 4:
		m_pFrontServer->SetLimit(8);
		break;
	case 5:
		m_pFrontServer->SetLimit(10);
		break;
	default:
		m_pFrontServer->SetLimit(6);
		break;
	}	

	if(!m_pFrontServer->Start(m_ListenPort))
	{
		OutputDebugString(L"DHT start failed! Something wrong with UDP");
	}

	while(!m_bStop)
	{
		m_pDealer->Dispatch();
	}

#ifdef WIN32
	//WSACleanup();
#endif
}

void CDHTThread::ReadNodes()
{

	//std::ifstream sfile(m_sPath,std::ifstream::in );
	//std::string line;

	//while(sfile.good())
	//{
	//	sfile>>line;
	//	size_t split=line.find(":");
	//	if(split==std::string::npos) continue;

	//	std::string ipstr=line.substr(0,split);
	//	std::string portstr=line.substr(split+1);				
	//	unsigned int iip=inet_addr(ipstr.c_str());
	//	unsigned short iport= (unsigned short)(atoi(portstr.c_str()));
	//	iport=htons(iport);
	//	if(iip==INADDR_NONE||iport==0) continue;
	//	AddInitialNodes(iip,iport);

	//}

	//sfile.close();

	
}

void CDHTThread::SaveNodes()
{
	//std::string nodes;
	//m_pMyNode->GetAllNodeData(nodes);

	//if(nodes.empty()) return;
	//
	//std::ofstream sfile(m_sPath, std::ios_base::trunc);

	//for(unsigned int i=0;i<nodes.size()/6;i++)
	//{
	//	//one line one ip:port
	//	std::string ipport=nodes.substr(i*6,6);
	//	std::string ipstr;
	//	unsigned short port;

	//	if(!Tools::unpactaddr(ipport,ipstr,port)) continue;

	//	sfile<<ipstr<<":"<<port<<std::endl;

	//}

	//sfile.close();

}

void CDHTThread::Stop()
{
	m_bStop=true;
	Wait();

	SaveNodes();

	delete m_pFrontServer;
	m_pFrontServer=NULL;
	delete m_pDealer;
	m_pDealer=NULL;
	delete m_pMyNode;
	m_pMyNode=NULL;
	delete m_pDataBase;
	m_pDataBase=NULL;
}

bool CDHTThread::Start(unsigned short port)
{
//init the job
	m_ListenPort=port;
	
	m_pDataBase=new CDataBase(this);
	m_pMyNode=new CDHTNode(this);
//set the slot limit by level
	switch(m_nLevel)
	{
	case 1:
		m_pMyNode->SetSlotLimit(4);
		break;
	case 2:
		m_pMyNode->SetSlotLimit(5);
		break;
	case 3:
		m_pMyNode->SetSlotLimit(6);
		break;
	case 4:
		m_pMyNode->SetSlotLimit(7);
		break;
	case 5:
		m_pMyNode->SetSlotLimit(8);
		break;
	default:
		m_pMyNode->SetSlotLimit(6);
		break;
	}

	m_pDealer=new SockLib::CDealer();
	
	m_bStop=false;

	ReadNodes();

	m_pMyNode->Start();
	m_pDataBase->Start();

	return Run(false); //joinable thread
}

SockLib::CDealer* CDHTThread::GetDealer()
{
	return m_pDealer;
}


const BTDHTKey& CDHTThread::GetMyID()
{
	return m_pMyNode->GetMyID();
}

void CDHTThread::ErrorPingResponse(unsigned int ip,  unsigned short port,BTDHTKey& key)
{

	SockLib::TInetAddr4 addr(ip,port);

	m_pMyNode->UpdateNoResponse(addr,key);
}

void CDHTThread::GoodPingResponse(unsigned int ip, unsigned short port, BTDHTKey &key)
{
//·�ɱ�ˢ�£�Ҳ������㲻��·�ɱ�������������ܲ���
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->Update(addr,key);
//�����ڵ�أ����������GET_PEERS
	m_pMyNode->AddActiveNode(key,addr);
}

void CDHTThread::NoResponse(unsigned int ip, unsigned short port,BTDHTKey& key)
{
//·�ɱ�ˢ�£�
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->UpdateNoResponse(addr,key);
//ˢ�½ڵ��
	m_pMyNode->RemoveActiveNode(key);
}

void CDHTThread::ErrorFindNodeResponse(unsigned int ip,  unsigned short port,BTDHTKey& key)
{
//·�ɱ�ˢ�£�
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->UpdateNoResponse(addr,key);
//�����ڵ�أ����������GET_PEERS
	m_pMyNode->AddActiveNode(key,addr);
}

void CDHTThread::GoodFindNodeResponse(unsigned int ip,  unsigned short port,BTDHTKey &key, std::string& peerStr,BTDHTKey &target)
{
//·�ɱ�ˢ�£�Ҳ������㲻��·�ɱ�������������ܲ���
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->Update(addr,key); //update the responser it's active
//�����ڵ�أ����������GET_PEERS
	m_pMyNode->AddActiveNode(key,addr);

	for ( unsigned int i = 0; i < peerStr.size() / 26; ++i )
	{
		BTDHTKey newkey( peerStr.data() + i * 26 );
		std::string ipport=peerStr.substr(i*26+20, 6);
		
		//add new peers
		SockLib::TInetAddr4 newaddr;
		newaddr.iip= *((unsigned int*)(ipport.data()));
		newaddr.iport= *((unsigned short*)(ipport.data()+4));		

		//only make find_node if the slot not full

		if(m_pMyNode->IsSlotFull(newkey)) {
			continue;
		}else if(m_pMyNode->IsContain(newkey)) {
			continue;
		}else if(!m_pMyNode->RecordNodeForFindNode(newaddr)){
			continue;
		}else{
			m_pMyNode->DoFindNodeOnNode(newaddr.iip,newaddr.iport,newkey,target);
		}

	}



}

void CDHTThread::ErrorGetPeersResponse(unsigned int ip,  unsigned short port, BTDHTKey& peerid)
{

	SockLib::TInetAddr4 addr(ip,port);

	m_pMyNode->Update(addr,peerid); //update the responser it's active
}

//may return value(peers) or closer node
void CDHTThread::GoodGetPeerResponse(unsigned int ip, unsigned short port, BTDHTKey &key, BTDHTKey& target, bool bepeers, std::string& peerStr)
{
//·�ɱ�ˢ�£�Ҳ������㲻��·�ɱ�������������ܲ���
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->Update(addr,key); //update the responser it's active
//�����ڵ�أ����������GET_PEERS
	m_pMyNode->AddActiveNode(key,addr);
	
	if(bepeers)
	{//value
	
		for ( unsigned int i = 0; i < peerStr.size() / 6; ++i )
		{
			CDBItem item((const unsigned char*)(peerStr.data() + i * 6));
			//the target is infohash in get_peers
			AddValueInfoOnHash(target, item);

		}

	}
	else
	{//closer node
		//target ��Ҫ�ҵ�infohash, ������������п����Ѿ�������ɾ����ȡ�����ڽ��еݹ�
		//����ǰ�����ж��Ƿ���������ڣ���������˾Ͳ���������
		if( m_pMyNode->IsTaskExists(target))
		{

			//�µĽڵ��ڻ�ڵ����������8λ֮ǰ�ſ��Ƕ������м���������Ҳû����
			CClosestNodeStore check(target,8);
			m_pMyNode->GetClosestGoodNodeInActivePool(check);

			//parse the node info we can get the node_id and ip,port
			for ( unsigned int i = 0; i < peerStr.size() / 26; ++i )
			{
				BTDHTKey newkey( peerStr.data() + i * 26 );

				//newkey Ҫ���ڻ�ڵ���о���Ŀ���������8λ�ڣ�����ѭ��Ҳû��˼
				if( !check.IsNearer(newkey) )
				{
					continue;
				}			

				//�µĽڵ�������ǵ�target�������ǲ�ȥѭ��������ֹͣѭ��
				if( (newkey - target) > (key - target) ) //��ȡ�Ⱥţ���ֹ�Է������а����Լ�����ѭ��
				{
					continue;
				}

				std::string ipport=peerStr.substr(i*26+20,6);

				//add new peers
				SockLib::TInetAddr4 newaddr;
				newaddr.iip= *((unsigned int*)(ipport.data()));
				newaddr.iport= *((unsigned short*)(ipport.data()+4));

				if(!m_pMyNode->IsNodeInActivePool(newkey))
				{//����Ѿ������ǳ��Ҫô�����ϵģ�Ҫô�����µ��´�ѭ������
						//������ϵģ�Ҫô������ѡ�ִ����ˣ�Ҫô�����ӽ�Ŀ�겻������
					m_pMyNode->DoGetPeersOnNode(newaddr.iip,newaddr.iport,newkey,target);
				}

			
			}
		}

	}

	
}

//����������
void CDHTThread::ErrorAnnouncePeersResponse(unsigned int ip, unsigned short port)
{
	//printf("ErrorAnnouncePeersResponse,ip=%s,port=%u\n",ip.c_str(),port);
}

void CDHTThread::GoodAnnouncePeersResponse(unsigned int ip, unsigned short port, BTDHTKey& key)
{
//����·�ɱ���
	SockLib::TInetAddr4 addr(ip,port);
	m_pMyNode->Update(addr,key);	
//�����ڵ�أ����������GET_PEERS
	m_pMyNode->AddActiveNode(key,addr);
	m_pMyNode->GoodAnnouncePeersResponse(ip,port,key);
}

unsigned short CDHTThread::GetListenPort()
{
	return m_ListenPort;
}

//it's a task that find all the peers for a given infohash
//after setinitialnodes call it to get the peers info
bool CDHTThread::AddTask(const char *infohash,bool announce,unsigned port)
{
	return m_pMyNode->AddGetPeerTask(infohash,announce, port);
}


bool CDHTThread::IsContain(BTDHTKey &key)
{
	return m_pMyNode->IsContain(key);
}

bool CDHTThread::GetEntryAddr(BTDHTKey &key, SockLib::TInetAddr4 &addr)
{
	return m_pMyNode->GetEntryAddr(key,addr);
}

//��·�ɱ���ȡ�����
void CDHTThread::GetClosestGoodNode(CClosestNodeStore &store)
{
	m_pMyNode->GetClosestGoodNode(store);
}

CDataBase* CDHTThread::GetDataBase()
{
	return m_pDataBase;
}

CFrontServer* CDHTThread::GetFrontServer()
{
	return m_pFrontServer;
}

void CDHTThread::AddValueInfoOnHash(BTDHTKey &key, CDBItem &item)
{
	//m_pDataBase->Store(key,item); //duplicate check in it
	m_pDataBase->StoreTaskValue(key,item);
}


//update the kbucket for last seen by node request
void CDHTThread::UpdateForRequest(SockLib::TInetAddr4 &addr, BTDHTKey &nodekey)
{
	m_pMyNode->Update(addr,nodekey);
}

void CDHTThread::AddInitialNodes(unsigned int ip, unsigned short port)
{

	SockLib::TInetAddr4 addr;
	addr.iip=ip;
	addr.iport=port;

	m_InitialNodeMutex.Lock();
	m_InitialNode.push_back(addr);
	m_InitialNodeMutex.Unlock();


}

void CDHTThread::RemoveTask(char *hash)
{
	m_pMyNode->RemoveGetPeersTask(hash);
	BTDHTKey hashkey(hash);
	m_pFrontServer->CancelTask(hashkey);
}

int CDHTThread::GetPeers(char *hash, int buflen, char *buf, int* total)
{
	return m_pDataBase->GetPeers(hash,buflen,buf,total);
}



void CDHTThread::DoAnnounceTask(std::string &tokenstr, std::string &infohash, unsigned int ip, unsigned short port)
{

	m_pMyNode->DoAnnounceJob(tokenstr,infohash,ip, port);
}

int CDHTThread::GetItemCount()
{
	return m_pMyNode->GetItemCount();
}

void CDHTThread::SetOptions(bool findpeer, bool announce, bool server,unsigned short level)
{
	m_bAnnounce=announce;
	m_bAsServer=server;
	m_bFindPeer=findpeer;
	m_nLevel=level;

	if(m_pFrontServer!=NULL)
	{
		switch(m_nLevel)
		{
		case 1:
			m_pFrontServer->SetLimit(4);
			break;
		case 2:
			m_pFrontServer->SetLimit(6);
			break;
		case 3:
			m_pFrontServer->SetLimit(8);
			break;
		case 4:
			m_pFrontServer->SetLimit(10);
			break;
		case 5:
			m_pFrontServer->SetLimit(12);
			break;
		default:
			m_pFrontServer->SetLimit(8);
			break;
		}	
		
	}

	if(m_pMyNode!=NULL)
	{
		switch(m_nLevel)
		{
		case 1:
			m_pMyNode->SetSlotLimit(5);
			break;
		case 2:
			m_pMyNode->SetSlotLimit(6);
			break;
		case 3:
			m_pMyNode->SetSlotLimit(7);
			break;
		case 4:
			m_pMyNode->SetSlotLimit(8);
			break;
		case 5:
			m_pMyNode->SetSlotLimit(8);
			break;
		default:
			m_pMyNode->SetSlotLimit(8);
			break;
		}
	}
}

//lport is task listen port
//uport is dht listen port ,used for check token
//return 0 is success
int CDHTThread::DoPeerAnnounceTask(std::string& token, std::string& hash, SockLib::TInetAddr4& iaddr, int lport )
{

	if(!m_bAsServer) 
	{//�Ѽ����Ч��
		return -1;
	}

	if(token.size() !=20) 
	{
		//ȷ����20�ֽڳ��ȣ��������������������ʵǰ���Ѿ������
		return -2;
	}

	BTDHTKey tokenkey(token.data()); 

	//CheckToken �����������IP��PORT
	if(!m_pDataBase->CheckToken(tokenkey,iaddr.iip,iaddr.iport))
	{
		return -3;
	}
	
	BTDHTKey hashkey(hash.data());

	CDBItem dbi(iaddr.iip,lport);
	
	if(!m_pDataBase->Store(hashkey,dbi))
	{
		return -5;
	}

	return 0;
}


void CDHTThread::AddFixNodes()
{
	/*
	router.bittorrent.com:6881
	dhtbootstrap.depthstrike.com:5560
	84.255.198.133:11513
	router.utorrent.com:6881
	ayu.depthstrike.com:21853
	router.bitcomet.com:554
	*/

	unsigned int iip=0;

	if(GetServerIP("router.bittorrent.com",iip))
	{
		AddInitialNodes(iip,htons(6881));
	}

	if(m_bStop) return; //��ֹ������ַ����ʱ��̫���������ܼ�ʱ�˳�

	if(GetServerIP("dhtbootstrap.depthstrike.com",iip))
	{
		AddInitialNodes(iip,htons(5560));
	}
	
	if(m_bStop) return;

	if(GetServerIP("router.utorrent.com",iip))
	{
		AddInitialNodes(iip,htons(6881));
	}	

	if(m_bStop) return;

	if(GetServerIP("ayu.depthstrike.com",iip))
	{
		AddInitialNodes(iip,htons(21853));
	}

	if(m_bStop) return;

	if(GetServerIP("router.bitcomet.com",iip))
	{
		AddInitialNodes(iip,htons(554));
	}


}

bool CDHTThread::GetServerIP(std::string server, unsigned int& iip)
{

//we parser the host and return the ip

	struct hostent *he;
	he=gethostbyname(server.c_str());
	if(he==NULL) return false;

	if ( he->h_addrtype != AF_INET )
			return false;

	struct in_addr *addr=( struct in_addr* ) ( he->h_addr );

#ifdef WIN32
	iip=addr->S_un.S_addr;
#else
	iip=addr->s_addr;
#endif

	return true;
}

std::string CDHTThread::GetIpString(unsigned int iip)
{
#ifdef WIN32
	const char *ret;
	ret = inet_ntoa( *((in_addr*)(&iip)) );
#else
	const char *ret;
	char ipbuf[ INET_ADDRSTRLEN ];
	ret = inet_ntop( AF_INET, (const void*)(&iip), ipbuf, INET_ADDRSTRLEN );
#endif

	return ret;
}

//get a init node return and remove it
bool CDHTThread::GetInitNode(SockLib::TInetAddr4& addr)
{
	SockLib::CAutoLock al(m_InitialNodeMutex);

	if(m_InitialNode.empty()) return false;

	addr=m_InitialNode.front();
	m_InitialNode.pop_front();

	return true;
}

// Implementation of missing methods
void CDHTThread::SetSavePath(const char* path)
{
	if (path != NULL) {
#ifdef WIN32
		strncpy_s(m_sPath, sizeof(m_sPath), path, _TRUNCATE);
#else
		strncpy(m_sPath, path, sizeof(m_sPath) - 1);
		m_sPath[sizeof(m_sPath) - 1] = '\0';
#endif
	}
}

void CDHTThread::SetLogBack(LOGBACK callback)
{
	m_logCallback = callback;
}

void CDHTThread::SetEventBack(SERVICEEVENT callback)
{
	m_eventCallback = callback;
}

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

// DataBase.cpp: implementation of the CDataBase class.
//
//////////////////////////////////////////////////////////////////////



#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include "../include/DataBase.h"
#include "../include/DHTThread.h"
#include <Dealer.h>
#include <AutoLock.h>
#include <SHA1.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#if defined(WINCE)
#define ITEM_LIMIT (3000)
#else
#define ITEM_LIMIT (20000)
#endif

extern void syslog(std::string info);

CDataBase::CDataBase(CDHTThread* parent)
{
	m_pParent=parent;
	m_nTotalItems=0;
	m_nTimer=0;
}

CDataBase::~CDataBase()
{

}

//store the <key,value> and avoid duplicate data
bool CDataBase::Store(const BTDHTKey & key,const CDBItem & dbi)
{

	if(m_nTotalItems >= ITEM_LIMIT) return false;

	std::map<BTDHTKey,DBItemList>::iterator it;

	SockLib::CAutoLock al(m_DatabaseMutex);

	it=m_Database.find(key);

	if(it==m_Database.end())
	{
		std::list<CDBItem> nl;
		nl.push_back(dbi);
		m_Database[key] = nl;
		//m_nTotalItems++;
	}
	else
	{
		//store must avoid duplicate item,if duplicate ,remove the old and insert new
		//because of time_stamp
		DBItemList::iterator it2;
		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			if((*it2)==dbi) {
				it->second.erase(it2);
				m_nTotalItems--;
				break;
			}
		}

		it->second.push_back(dbi);
	}

	m_nTotalItems++;

	return true;
}

//get some random item return
void CDataBase::Sample(const BTDHTKey & key,DBItemList & dbl,unsigned int max)
{
	std::map<BTDHTKey,DBItemList>::iterator it;

	it=m_Database.find(key);

	if(it==m_Database.end())
	{
		return;
	}
		
	if (it->second.size() < max)
	{
		DBItemList::iterator it2;
		
		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			dbl.push_back(*it2);
		}
	
	}
	else
	{
		unsigned int count= 0;
		DBItemList::iterator it2;
		
		int total=it->second.size();
		int ratio= int(100.0*float(max)/float(total)); //0-99


		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			if( rand()%100 <= ratio) 
			{
				dbl.push_back(*it2);
				if(count++ >= max) return;
			}
			else
			{
				continue;
			}

		}

	}
}

//expire the items
void CDataBase::Expire(unsigned int now)
{
	std::map<BTDHTKey,DBItemList>::iterator it;

	SockLib::CAutoLock al(m_DatabaseMutex);

	for(it=m_Database.begin();it!=m_Database.end();)
	{
		DBItemList::iterator it2;
		for(it2=it->second.begin();it2!=it->second.end();)
		{
			if(it2->IsExpired(now))
			{
				it->second.erase(it2++);
				m_nTotalItems--;
				continue;
			}
			
			it2++;
		}

		//ɾ���հ׵�HASH�б�
		if(it->second.empty())
		{
			m_Database.erase(it++);
			continue;
		}

		it++;
	}

}


//ip and port ����������
bool CDataBase::CheckToken( BTDHTKey &token,unsigned int ip,unsigned short port)
{
	// the token must be in the map
	if (m_TokenMap.find(token)==m_TokenMap.end())
	{
		return false;
	}


	// in the map so now get the timestamp and regenerate the token
	// using the IP and port of the sender
	unsigned int ts = m_TokenMap[token];

	unsigned char tdata[20];
	
	memcpy(tdata,&ip,4);
	memcpy(tdata+4,&port,2);
	memcpy(tdata+6,&ts,sizeof(unsigned int));
	
	//unsigned char hash[20];
	//SHA1Block(tdata,2+4+sizeof(unsigned int),hash);
	//BTDHTKey ct((const char*)hash);

	HashLib::CSHA1 haobj;
	haobj.Hash((const char*)tdata, 2+4+sizeof(unsigned int));
	
	BTDHTKey ct((const char*)haobj.GetHash());
	

	
	if (token != ct)  
	{
		return false;
	}

	m_TokenMap.erase(token);

	return true;
}


void CDataBase::Start()
{
	m_nTimer=m_pParent->GetDealer()->AddTimer(this,10*1000);
}

void CDataBase::OnTimer(unsigned int id)
{
	if(m_nTimer==id)
	{

		unsigned int now=GetTickCount();
		
		Expire(now);

		//clean token

		TTokenMap::iterator it;

		for(it=m_TokenMap.begin();it!=m_TokenMap.end();)
		{
			if(now < it->second) //tick count ת��һȦ
			{
				m_TokenMap.erase(it++);	
			}
			else if( now - it->second > 30*1000)
			{//����30��
				m_TokenMap.erase(it++);
			}
			else
			{
				it++;
			}
		}

	}
}

int CDataBase::GetItemCount()
{
	return m_nTotalItems;
}


//��������
BTDHTKey CDataBase::GenToken(sockaddr_in &addr)
{
	unsigned char tdata[20];
	unsigned int now = GetTickCount();
	// generate a hash of the ip port and the current time
	// should prevent anybody from crapping things up
	
	//SHA1(ip+port+time)== token
	memcpy(tdata,&addr.sin_addr,4); //sin_addr is 4 byte
	memcpy(tdata+4,&addr.sin_port,2); //unsigned short
	memcpy(tdata+2+4,&now,sizeof(unsigned int));
	
	//unsigned char hash[20];
	//SHA1Block(tdata,2+4+sizeof(unsigned int),hash);
	
	//BTDHTKey token((const char*)hash);
	HashLib::CSHA1 haobj;
	haobj.Hash((const char*)tdata,2+4+sizeof(unsigned int));

	BTDHTKey token((const char*)haobj.GetHash());

	//����30�����
	m_TokenMap[token]=now;

	return token;
}

//�����������������ʵ��Ŀǰ���������
bool CDataBase::CheckToken( BTDHTKey &token, sockaddr_in &addr)
{

	unsigned short port=addr.sin_port;
	unsigned int ip= *((unsigned int*)(&addr.sin_addr));

	return CheckToken(token,ip,port);

}

void CDataBase::Stop()
{
	if(m_nTimer!=0)
	{
		m_pParent->GetDealer()->RemoveTimer(m_nTimer);
		m_nTimer=0;
	}

	//clean the database?

}


//get peers may called by other thread ,so lock database
int CDataBase::GetPeers(char *infohash, int buflen, char *buf, int* total)
{

	BTDHTKey hash(infohash);

	std::map<BTDHTKey,DBItemList>::iterator it;

	m_TaskDatabaseMutex.Lock();

	it=m_TaskDatabase.find(hash);

	if(it==m_TaskDatabase.end())
	{
		*total=0;
		m_TaskDatabaseMutex.Unlock();
		return 0;
	}


	int bmax=buflen/6;
	*total=it->second.size();

	if(bmax<=0)
	{
		m_TaskDatabaseMutex.Unlock();
		return 0;
	}

	DBItemList::iterator it2;
	int counter=0;

	for(it2=it->second.begin();it2!=it->second.end();)
	{

		memcpy(buf+6*counter,it2->GetData(),6);

		it->second.erase(it2++);  //delete the transfered value, have problem in PPC?
		//it2=it->second.erase(it2);

		if(++counter >= bmax) break;
	}
	
	m_TaskDatabaseMutex.Unlock();
	return counter;


}

void CDataBase::StoreTaskValue(const BTDHTKey &key, const CDBItem &dbi)
{
	std::map<BTDHTKey,DBItemList>::iterator it;

	m_TaskDatabaseMutex.Lock();

	it=m_TaskDatabase.find(key);

	if(it==m_TaskDatabase.end())
	{
		std::list<CDBItem> nl;
		nl.push_back(dbi);
		m_TaskDatabase[key] = nl;
	}
	else
	{
		//store must avoid duplicate item,if duplicate ,remove the old and insert new
		//because of time_stamp
		DBItemList::iterator it2;
		for(it2=it->second.begin();it2!=it->second.end();it2++)
		{
			if((*it2)==dbi) {
				it->second.erase(it2);
				break;
			}
		}

		it->second.push_back(dbi);
	}

	m_TaskDatabaseMutex.Unlock();
}


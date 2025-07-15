/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// DataBase.h: interface for the CDataBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DATABASE_H
#define _DATABASE_H

#include "DBItem.h"
#include "BTDHTKey.h"

#if defined(WINCE)
#include <windows.h>
#elif define( WIN32)
#pragma warning(disable: 4786)
#include <winsock2.h>
#else
// For Windows, use winsock2.h instead of Unix socket headers
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
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
#endif

#include <Mutex.h>
#include <Tools.h>
#include <TimerClient.h>
#include <list>
#include <map>

class CDHTThread;
class CDataBase : public SockLib::CTimerClient 
{
	typedef std::list<CDBItem> DBItemList;
	typedef std::map<BTDHTKey,unsigned int> TTokenMap;

public:
	void StoreTaskValue(const BTDHTKey& key, const CDBItem& dbi);
	int GetPeers(char* hash, int buflen, char* buf, int* total);
	void Stop();
	bool CheckToken( BTDHTKey& token,sockaddr_in &addr);
	BTDHTKey GenToken(sockaddr_in& addr);
	virtual void OnTimer(unsigned int id);
	void Start();
	CDataBase(CDHTThread* parent);
	virtual ~CDataBase();
	bool Store(const BTDHTKey & key,const CDBItem & dbi);
	void Sample(const BTDHTKey & key,DBItemList & dbl,unsigned int max);
	void Expire(unsigned int now);
	bool CheckToken( BTDHTKey & token,unsigned int ip,unsigned short port);
	int GetItemCount();
private:
	CDHTThread* m_pParent;
	SockLib::CMutex		m_DatabaseMutex;

	std::map<BTDHTKey,DBItemList> m_Database;

	//the taskdatabase store the values for our running task, not store the value from other peer request
	SockLib::CMutex		m_TaskDatabaseMutex;
	std::map<BTDHTKey,DBItemList> m_TaskDatabase;

	TTokenMap m_TokenMap;;
	int m_nTotalItems;
	unsigned int m_nTimer;


};

#endif

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// UPnpThread.h: interface for the CUPnpThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _UPNP_THREAD_H
#define _UPNP_THREAD_H
#ifdef WIN32
#include <winsock2.h>
#elif defined(WINCE)
#include <windows.h>
#endif

#include <ThreadBase.h>
#include <Mutex.h>
#include <TimerClient.h>
#include <Dealer.h>
#include <string>

class CUPnpNat;
class CDealer;
class CUPnpNat;
class CUPnpThread : public SockLib::CThreadBase, public SockLib::CTimerClient
{
public:
	bool IsExternIp(std::string ip);
	bool IsLoopIp(std::string ip);
	void addPortMap( unsigned int port, std::string protocol );
	void removePortMap( unsigned int port, std::string protocol );
	bool GetExternIP( char* ipbuf);
	void Stop();
	bool Start();
	CUPnpThread();
	virtual ~CUPnpThread();
	virtual void OnTimer(unsigned int id);
protected:
	bool FindLocalForExternIP(std::string& xip);
	virtual void Entry();
	bool IsInnerIP( std::string ip );
private:
	bool			m_bStop;
	SockLib::CDealer*		m_pDealer;
	CUPnpNat*		m_pUPnpNat;
	std::string		m_sExternIP;
	bool			m_bInnerIP;
	bool			m_bGotIP;
public:
	void NoticeExternIP(std::string ipstr);
};

#endif

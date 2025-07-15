/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _UPNPNAT_H
#define _UPNPNAT_H

#include <string>
#include <list>

#include <Sock.h>
#include <TimerClient.h>

#include "UPnpNatFinderHandler.h"
#include "UpnpNatParserHandler.h"
#include "UPnpNatFinder.h"
#include "UPnpNatParser.h"
#include "UPnpNatController.h"


enum TUPnpNATState{NS_INIT, NS_DISCOVER, NS_GETDESCRIPTION, NS_OK, NS_ERROR} ;

class CUPnpNatController;

struct TNatTaskInfo
{
    unsigned int port;
    std::string protocol;
    TUPnpAction action;
	unsigned int retry;
    CUPnpNatController* controller;
};

typedef std::list<TNatTaskInfo> TNatTaskList;

class CUPnpThread;
class CUPnpNat :
			public SockLib::CTimerClient,
            public CUPnpNatFinderHandler,
            public CUPnpNatParserHandler
{

public:
    CUPnpNat(CUPnpThread* parent);
    virtual ~CUPnpNat();

private:
    CUPnpNatFinder _natExplorer;
    CUPnpNatParser _natParser;

	SockLib::CDealer*	m_pDealer;

	CUPnpThread*	m_pParent;

    TUPnpNATState	_state;

    std::string _service;
	std::string m_sDescriptionUrl;
	std::string m_sControlUrl;

    TNatTaskList m_TaskList;

    unsigned int m_discoverTimerID;
    unsigned int _controlTimerID;
	unsigned int m_nCheckIpTimer;
	unsigned int m_nFailTime;
    std::string m_sExternIP;

public:
    void NoticePortMapping( bool ok, bool addport, int inport, std::string protocol,int ncode );
    void NoticeExternIP(  bool ok,std::string ip ,int ncode);

    void GetExternIp();
    void SetDealer( SockLib::CDealer* dealer );
    void Start();
    void Stop();
    TUPnpNATState getState();
    void addPortMapping( unsigned int port, std::string protocol );
    void removePortMapping( unsigned int port, std::string protocol );
//	void LogMsg(wchar_t* msg,_MSGTYPE type);
    virtual void OnTimer( unsigned int id );

    void OnGetDescriptionUrl( std::string&  url  );
    void OnGetControlUrl( const char* controlUrl, const char* service );
};

#endif /*UPNPNAT_H_*/

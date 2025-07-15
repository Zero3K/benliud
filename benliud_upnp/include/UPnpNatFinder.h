/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _UPNPNATEXPLORER_H
#define _UPNPNATEXPLORER_H

#include <UDPClientSock.h>
#include <TimerClient.h>

#include <vector>

class CSockDealer;
class CTimerDealer;
class CUPnpNatFinderHandler;

class CUPnpNatFinder :
            public SockLib::CUDPClientSock

{

public:
    CUPnpNatFinder();
    virtual ~CUPnpNatFinder();

private:
   
    CUPnpNatFinderHandler* _eventHandler;
    unsigned int _discoverTimerID;
    bool _found;
    int m_nTryTimes;
    std::vector<std::string> m_IpList;
	SockLib::CDealer*	m_pDealer; 
private:
    void Discover();
    void ParseReponse( std::string& content, int len );

public:

    bool IsInnerIp( std::string ip );
    void MakeIpList();
    void setEventHandler( CUPnpNatFinderHandler* handler );
    void Start();
    void Stop();

    virtual void OnRead();
    virtual void OnWrite();

    virtual void OnTimer( unsigned int id );

protected:
    std::string GetLocalHost();
};

#endif


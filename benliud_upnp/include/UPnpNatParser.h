/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _UPNPNATPARSER_H
#define _UPNPNATPARSER_H

#include <TCPClientSock.h>

enum TDescriptionParserState{DPS_INIT, DPS_WORKING, DPS_OK, DPS_ERROR} ;

class CUPnpNatParserHandler;
class CUPnpNatParser :
            public SockLib::CTCPClientSock
{

public:
    CUPnpNatParser();
    virtual ~CUPnpNatParser();

private:

    SockLib::CDealer* m_pDealer;
    std::string _url;
    CUPnpNatParserHandler* _eventHandler;
    std::string _recvBuffer;
    TDescriptionParserState _state;

private:

    void sendRequest();
    bool ParseResponse( std::string&  response, const char* httpUrl, const char* serviceType );
public:

    void setDescriptionUrl( std::string& url );
    void setEventHandler( CUPnpNatParserHandler* handler );

    void Start();
    void Stop();

    TDescriptionParserState getState();
	bool parseUrl( const char* url, std::string& host, unsigned short& port, std::string& path );

    virtual void OnRead();
    virtual void OnWrite();
    virtual void OnClose();
	virtual void OnConnectFail();
	virtual void OnConnectOk();

    virtual void OnTimer( unsigned int id );
};

#endif


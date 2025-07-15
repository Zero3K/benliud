
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _UPNPNATCONTROLLER_H
#define _UPNPNATCONTROLLER_H

#include <TCPClientSock.h>


enum TControllerState{CS_INIT, CS_WORKING, CS_OK, CS_ERROR} ;

enum TUPnpAction{AC_ADDPORT, AC_DELPORT, AC_GETIP} ;

class CUPnpNat;
class CDealer;
class CUPnpNatController : public SockLib::CTCPClientSock
{

public:
    CUPnpNatController();
    virtual ~CUPnpNatController();

private:

    TControllerState _state;
    std::string _controlUrl;
    std::string _service;
    unsigned short _port;
    std::string _protocol;
    std::string _actionName;

    CUPnpNat* m_pParent;
    TUPnpAction _action;
    std::string _localIP;
    std::string _recvBuffer;

private:
    void sendAddRequest();
    void sendGetipRequest();
    void sendDelRequest();

public:
    void SetParent( CUPnpNat* parent );
    void setControlUrl( const char*controlUrl, const char* service );
    void setAction( TUPnpAction act, unsigned short port, std::string protocol );
    void start();
    void stop();
    TControllerState getState();

    virtual void OnRead();
    virtual void OnWrite();
    virtual void OnClose();
	virtual void OnConnectFail();
	virtual void OnConnectOk();
    virtual void OnTimer( unsigned int id );
	bool parseUrl( const char* url, std::string& host, unsigned short& port, std::string& path );

};

#endif


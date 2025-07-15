/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _SPEEDCONTROL_H
#define _SPEEDCONTROL_H


#include <list>

#ifdef WIN32
#include <winsock2.h>
#endif
#include <Mutex.h>
#include "SpeedControlBase.h"

class CBTPeer;



class CBTSession;

class CSpeedControl : public CSpeedControlBase
{
	typedef std::list<CBTPeer*> TClientList;

public:
    CSpeedControl( CBTSession* parent );
    virtual ~CSpeedControl();

private:

    TClientList m_ClientList;

	SockLib::CMutex m_PendingListMutex;
	TClientList m_PendingList;

	SockLib::CMutex m_RemoveMutex;
    //CMutex m_ClientListMutex;

    CBTSession* m_pParent;

private:
    void Upload();
    void Download();

public:
    void RegisteClient( CBTPeer* client );
    void UnregisteClient( CBTPeer* client );

    void Update();


protected:
    void CleanClient();
};

#endif


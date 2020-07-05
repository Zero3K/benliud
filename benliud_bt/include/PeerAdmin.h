
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _PEERADMIN_H
#define _PEERADMIN_H


#include <TimerClient.h>
#include "PeerAdminBase.h"

#include "BTPieceSum.h"
#ifdef WIN32
#include <winsock2.h>
#endif
#include <Mutex.h>

#include <string>
#include <list>
#include <Dealer.h>

#include "../../benliud/bittorrent_types.h"
//#include "../../benliud/include/msgtype_def.h"


class CBTSession;
class CBTPiece;
class CSpeedControlBase;


class CPeerAdmin : public SockLib::CTimerClient , public CPeerAdminBase
{
 	typedef std::list<CBTPeer*> TPeerList;
public:
    CPeerAdmin( CBTSession* parent, bool wefinish=false );
    virtual ~CPeerAdmin();
	bool GotHash(std::string hash, CBTPeer* client);
	SockLib::CDealer*  GetDealer();
private:

    CBTSession* m_pSession;

    unsigned int m_ConnectCheckTimer;
    unsigned int m_ChokeCheckTimer;

	_BT_ENCMODE	m_EncryptionMode;

	int		m_nTotalPeers;
	bool	m_bDownloadFinish;
	bool	m_bSwitchMode;

	TPeerList m_ConnectingPeerList;

    SockLib::CMutex m_ConnectedPeerListMutex;
	TPeerList m_ConnectedPeerList;

protected:
	void CheckPeerConnectionWhenNotFinish();	
	void CheckPeerConnectionWhenFinished();
	int  CloseInterestedPeer(int maxnum);
	void CloseTimeoutConnection();
public:
	bool GotEncryptHash(std::string hashxor, MSE::BigInt dhsecrect,CBTPeer* client);
	bool TransferPeer(CBTPeer* peer);
	CSpeedControlBase* GetSpeedControl();
	void SetEncryptionMode(_BT_ENCMODE mode);
	void CheckClosedConnection();
	void LaunchNewConnectionWhenNotFinish();
    CBTSession* GetSession();
    bool Start();
    void Stop();

    void OnTimer( unsigned int id );

    void BroadcastNewPiece( int index );
    void DownloadFinish(bool finish=true);

protected:
	int CloseLowPriorityUploadPeer(int count);

#ifdef _CHECK
	void Debug();
//	void OutMsg(wchar_t* msg, _MSGTYPE type=MSG_INFO);
#endif
	void SwitchMode();
	void LaunchNewConnectionWhenFinished();
	void CheckConnectedConnecting();
	void CheckClosedConnecting();
	int CloseNotInterestPeer(int maxnum);
    void CheckPeerConnection();
    void AdjustChoke();

};

#endif

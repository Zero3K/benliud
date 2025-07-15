/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

// BTListener.h: interface for the CBTListener class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTLISTENER_H
#define _BTLISTENER_H

#include <windows.h> //wince need it

#include "../../benliud/bittorrent_types.h"
#include "datatype_def.h"
#include <ThreadBase.h>
#include <Dealer.h>
#include <Mutex.h>
#include <Tools.h>
#include <TimerClient.h>

#include "MSE_BigInt.h"

#include <map>

class CDealer;

class CBTListenSocket;

class CBTListenPeerAdmin;

class CSpeedControlBase;

class CBTListenSpeedControl;

class CBTStorage;

class CBTPeer;

class CBTListener : public SockLib::CThreadBase 
{
	typedef std::map<std::string, CBTStorage*> TJobMap;

public:
	void NewAccept(int handle, unsigned int iip, unsigned short iport);
	void SetDownloadSpeedLimit(int speed);
	void SetUploadSpeedLimit(int speed);
	int GetLeftDownBytes();
	int GetLeftUpBytes();
	int RunOffUpBytes(int bytes);
	int RunOffDownBytes(int bytes);
	bool LinkGotEncryptHash(std::string hashxor, MSE::BigInt S, CBTPeer* peer);
	
	//void SetEventBack( BTTASKEVENT eventback);
	unsigned int GetListenPort();
	void UnregisteTask(std::string hash );
	void RegisteTask(std::string hash, CBTStorage* task);
	bool LinkGotHash(std::string hash,CBTPeer* peer);
	CSpeedControlBase* GetSpeedControl();

	SockLib::CDealer* GetDealer();
	void Stop();
	bool Start(unsigned int port);
	CBTListener();
	virtual ~CBTListener();

protected:
	std::string HashXor(std::string hash, MSE::BigInt S);
	void Entry();
	bool m_bStop;
	unsigned int			m_nListenPort;
	SockLib::CDealer*				m_pDealer;
	CBTListenSocket*		m_pListenSocket;
	CBTListenPeerAdmin*		m_pListenPeerAdmin;
	CBTListenSpeedControl*	m_pListenSpeedControl;
	//BTTASKEVENT				m_pTaskEvent;
	//UPNPSERVICE				m_pUpnpService;
	SockLib::CMutex			m_JobMapMutex;
	TJobMap					m_JobMap;

//	CMutex					m_UploadSpeedMutex;
//	CMutex					m_DownloadSpeedMutex;

	int						m_nGlobalUploadSpeedLimit;
	int						m_nGlobalDownloadSpeedLimit;
	SockLib::CMutex			m_nGlobalUpByteCountMutex;
	int						m_nGlobalUpByteCount;
	SockLib::CMutex			m_nGlobalDownByteCountMutex;
	int						m_nGlobalDownByteCount;
};

#endif // !defined(AFX_BTLISTENER_H__897A5D0A_D0D1_4F0F_B301_9237CD1181BF__INCLUDED_)

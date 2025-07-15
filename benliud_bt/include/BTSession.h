
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
#ifndef _BTSESSION_H
#define _BTSESSION_H

#include <string>
#include <list>


#include "../../benliud/bittorrent_types.h"
#include <ThreadBase.h>
#include <Tools.h>
#include <TimerClient.h>
#include <TorrentFile.h>



#include "BTPiece.h"
#include "BTPeer.h"
#include "TTrackerDefines.h"	// Added by ClassView



class CTorrentFile;

class CTrackerManager;

class CPeerAdmin;

class CDealer;

class CBTStorage;

class CBTStorage;

class CPeerListener;

class CSpeedControl;

class CBTPeer;

class CBTSession : public SockLib::CThreadBase
{
	typedef std::list<unsigned int> TSpeedList;
public:
    CBTSession(bool wefinish=false);
    virtual ~CBTSession();
	BencodeLib::CTorrentFile* GetTorrentFile();
	//为中央集中管理PEER数据的接口
	bool GetPeerInfoToLink(
		bool connectable, 
		unsigned int& iip, 
		unsigned short& iport, 
		int& encref, 
		unsigned int& timeout);

	//为中央集中管理PEER数据的接口
	bool TryAcceptPeerLink(unsigned int iip);

	//为中央集中管理PEER数据的接口
	void LinkReport(unsigned int iip, bool ok);

	//为中央集中管理PEER数据的接口
	void CloseReport(unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset);
	//为中央集中管理PEER数据的接口
	void GiveUpLink(unsigned int iip);
	//为中央集中管理PEER数据的接口
	int CheckBitSet(std::string& peerid, unsigned int iip, CBTPiece& bitset);
	//为中央集中管理PEER数据的接口
	void LinkOkButNoRoomClose(unsigned int iip);

	bool AnyUnCheckedNode();
private:
    CBTStorage*		m_pStorage;
    CPeerAdmin*		m_pPeerAdmin;
	SockLib::CDealer*		m_pDealer;
    CSpeedControl*	m_pSpeedCtrl;


    unsigned int	m_nSessionId;
    bool			m_bStop;
	bool			m_bWeFinish;

	_BT_ENCMODE m_EncMode;
	llong m_nSumOfDownload;
	llong m_nSumOfUpload;

public:
	void PeerSupportEncryption(unsigned int iip, bool enc);
	void LinkOkButPeerClose(unsigned int iip);
	void GiveUpAcceptPeerLink(unsigned int iip);


	void DownloadFinish(bool finish=true);
	bool TransferPeer(CBTPeer* peer);
	void SetEncryptMode(_BT_ENCMODE mode);

	bool IsSelfPeerId(std::string& peerid);
	llong GetUploaded();
	llong GetDownloaded();
	void SumUpload(unsigned int iip, int bytes);
	void SumDownload(unsigned int iip, int bytes);
	CBTPiece& GetBitSet();
	//bool NewPeerNotice( const char *ip, unsigned int port );
	int GetSessionID();
	void Wait();
    void BroadcastNewPiece( int index );

    void SetStorage( CBTStorage* pStorage );
    bool Start();
    void Stop();
    const char* GetMyID();

    SockLib::CDealer* GetDealer();
    CPeerAdmin* GetPeerAdmin();
    CBTStorage*	GetStorage();
	CSpeedControl* GetSpeedControl();
    unsigned int GetLinkMax();
    unsigned int GetUploadLinkMax();
    unsigned int GetConnectingMax();
#ifdef _CHECK
    //void LogMsg( wchar_t* msg, _MSGTYPE type=MSG_INFO );
#endif
protected:
    virtual void Entry();

};

#endif

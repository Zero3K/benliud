
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

#if defined(WIN32)||defined(WINCE)
#pragma warning (disable: 4786)
#else
#include <netdb.h> //gethostbyname
#include <arpa/inet.h>  //inet_ntoa
#endif


#include "../include/BTSession.h" 
#include <TorrentFile.h>
#include <Dealer.h>
#include "../include/PeerAdmin.h"
#include "../include/BTStorage.h"
#include "../include/SpeedControl.h"

#include <assert.h>

extern void syslog(std::string info);

CBTSession::CBTSession(bool wefinish)
{
    m_pPeerAdmin = NULL;
    m_pStorage = NULL;
    m_pDealer = NULL;
    m_pSpeedCtrl = NULL;
	m_nSumOfDownload = 0; 
	m_nSumOfUpload = 0;
	m_nSessionId = 0;
	m_bWeFinish=wefinish;
}

CBTSession::~CBTSession()
{
  
	if(m_pPeerAdmin!=NULL)
	{
		delete m_pPeerAdmin;
	}

	if(m_pDealer!=NULL)
	{
		delete m_pDealer;
	}
	
	if(m_pSpeedCtrl!=NULL)
	{
		delete m_pSpeedCtrl;
	}


}


bool CBTSession::Start()
{

    m_bStop = false;
	m_nSumOfDownload = 0; 
	m_nSumOfUpload = 0;

    //assert ( m_pStorage != NULL );

	//assert(m_pDealer==NULL);
	m_pDealer=new SockLib::CDealer();

	//assert(m_pSpeedCtrl==NULL);
    m_pSpeedCtrl = new CSpeedControl( this );

	//assert(m_pPeerAdmin==NULL);
    m_pPeerAdmin = new CPeerAdmin( this, m_bWeFinish );
	m_pPeerAdmin->SetEncryptionMode(m_EncMode);

	m_nSessionId = m_pStorage->RegisteSession( this );

    if ( !m_pPeerAdmin->Start() )
    {
        return false;
    }

    return Run( false ); //a joinable thread start
}

void CBTSession::Stop()
{
    m_bStop = true;
}

BencodeLib::CTorrentFile* CBTSession::GetTorrentFile()
{
	//assert (m_pStorage!=NULL);
    return m_pStorage->GetTorrentFile();
}

const char* CBTSession::GetMyID()
{
	//assert (m_pStorage!=NULL);
	return m_pStorage->GetMyID(m_nSessionId);
}

SockLib::CDealer* CBTSession::GetDealer()
{
	//assert (m_pDealer!=NULL);
    return m_pDealer;
}

CPeerAdmin* CBTSession::GetPeerAdmin()
{
	//assert (m_pPeerAdmin!=NULL);
    return m_pPeerAdmin;
}


CBTStorage* CBTSession::GetStorage()
{
	//assert (m_pStorage!=NULL);
    return m_pStorage;
}


unsigned int CBTSession::GetLinkMax()
{
	//assert (m_pStorage!=NULL);
    return m_pStorage->GetLinkMax();

}

unsigned int CBTSession::GetUploadLinkMax()
{
	//assert (m_pStorage!=NULL);
	return m_pStorage->GetUploadLinkMax();
}


unsigned int CBTSession::GetConnectingMax()
{
	//assert (m_pStorage!=NULL);
	return m_pStorage->GetConnectingMax();
}


CSpeedControl* CBTSession::GetSpeedControl()
{
 	//assert (m_pSpeedCtrl!=NULL);
    return m_pSpeedCtrl;
}

void CBTSession::Entry()
{

#ifdef WIN32
    WSADATA wsadata;
    WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
#endif

    while ( !m_bStop )
    {
	
		m_pDealer->Dispatch();
		m_pSpeedCtrl->Update();	

    }


	
    if ( m_pStorage )
	{
		m_pStorage->UnregisteSession( this );
	}


    m_pPeerAdmin->Stop();  //no need wait



#ifdef WIN32
    //WSACleanup();
#endif
    return ;
}

//before session start, the storage should be set by calling it
//so, many session will have common storage.
//void CBTSession::SetStorage(CBTStorageBase *pStorage)
void CBTSession::SetStorage( CBTStorage* pStorage )
{
    m_pStorage = pStorage;
    //m_nSessionId = m_pStorage->RegisteSession( this, m_PeerID );
}


//up-to-down broadcast piece
void CBTSession::BroadcastNewPiece( int index )
{
	assert(m_pPeerAdmin!=NULL);
	m_pPeerAdmin->BroadcastNewPiece( index );
}


#ifdef _CHECK
//void CBTSession::LogMsg( wchar_t* msg, _MSGTYPE type )
//{
//	assert (m_pStorage!=NULL);
//    m_pStorage->LogMsg( msg, m_nSessionId,type );
//}
#endif

void CBTSession::Wait()
{
	CThreadBase::Wait();
}

int CBTSession::GetSessionID()
{
	return m_nSessionId;
}

void CBTSession::DownloadFinish(bool finish)
{
	m_bWeFinish=finish;
	if (m_pPeerAdmin!=NULL)
	{
		m_pPeerAdmin->DownloadFinish(finish);
	}
}

CBTPiece& CBTSession::GetBitSet()
{
	return m_pStorage->GetBitSet();
}

void CBTSession::SumDownload(unsigned int iip, int bytes)
{
	m_nSumOfDownload+=llong(bytes);
	//assert (m_pStorage!=NULL);
	m_pStorage->SumUpDownload( iip, bytes );
}

void CBTSession::SumUpload(unsigned int iip, int bytes)
{
	m_nSumOfUpload+=llong(bytes);
	//assert (m_pStorage!=NULL);
	m_pStorage->SumUpUpload( iip, bytes );
}

llong CBTSession::GetDownloaded()
{
	return m_nSumOfDownload;
}

llong CBTSession::GetUploaded()
{
	return m_nSumOfUpload;
}

bool CBTSession::IsSelfPeerId(std::string& peerid)
{
	return m_pStorage->IsSelfPeerId(peerid);
}



void CBTSession::SetEncryptMode(_BT_ENCMODE mode)
{
	m_EncMode=mode;
	if (m_pPeerAdmin!=NULL)
	{
		m_pPeerAdmin->SetEncryptionMode(m_EncMode);
	}
}

//���ڼ���ת�ƹ��������ӣ����ܾͷ����棬���ܽ��ܷ��ؼ�
bool CBTSession::TransferPeer(CBTPeer *peer)
{
	if(m_pPeerAdmin==NULL) {
		return false;
	}

	return m_pPeerAdmin->TransferPeer(peer);
}


//Ϊ���뼯�й���PEER���ݵĽӿ�
bool CBTSession::GetPeerInfoToLink(bool connectable, unsigned int &iip, unsigned short& iport, int& encref, unsigned int& timeout)
{
	return m_pStorage->GetPeerInfoToLink(m_nSessionId,connectable,iip,iport,encref,timeout);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
bool CBTSession::TryAcceptPeerLink( unsigned int iip)
{
	return m_pStorage->TryAcceptPeerLink(m_nSessionId,iip);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
void CBTSession::LinkReport(unsigned int iip, bool ok)
{
	m_pStorage->LinkReport(m_nSessionId,iip,ok);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
void CBTSession::CloseReport( unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset)
{
	m_pStorage->CloseReport(m_nSessionId,iip,reason,accepted,peerid,bitset);
}

//Ϊ���뼯�й���PEER���ݵĽӿ�
void CBTSession::GiveUpLink(unsigned int iip)
{	
	m_pStorage->GiveUpLink(m_nSessionId,iip);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
int CBTSession::CheckBitSet(std::string& peerid, unsigned int iip, CBTPiece& bitset)
{
	return m_pStorage->CheckBitSet(m_nSessionId,peerid,iip,bitset);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
void CBTSession::LinkOkButNoRoomClose(unsigned int iip)
{
	m_pStorage->LinkOkButNoRoomClose(m_nSessionId,iip);
}
//Ϊ���뼯�й���PEER���ݵĽӿ�
void CBTSession::GiveUpAcceptPeerLink(unsigned int iip)
{
	m_pStorage->GiveUpAcceptPeerLink(m_nSessionId, iip);
}

void CBTSession::LinkOkButPeerClose(unsigned int iip)
{
	m_pStorage->LinkOkButPeerClose(m_nSessionId,iip);
}

bool CBTSession::AnyUnCheckedNode()
{
	return m_pStorage->AnyUnCheckedNode();
}

//����ͨ�������������Է��Ƿ�֧�ּ���
void CBTSession::PeerSupportEncryption(unsigned int iip, bool enc)
{
	m_pStorage->PeerSupportEncryption(iip,enc);
}

//DEL void CBTSession::GotChunk(unsigned int iip)
//DEL {
//DEL 	m_pStorage->GotChunk(iip);
//DEL }

//DEL void CBTSession::SendChunk(unsigned int iip)
//DEL {
//DEL 	m_pStorage->SendChunk(iip);
//DEL }

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

#if defined (WIN32)||defined(WINCE)

#pragma warning (disable: 4786)

#include<stdlib.h> 
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <math.h>

#if !defined(WINCE)
#include <errno.h>
#include <assert.h>
#endif

#include "../include/BTPeer.h"
#include "../include/PeerAdminBase.h" 
#include "../include/SpeedControlBase.h"
#include "../include/BTStorage.h"
#include "../include/BTSession.h"
#include <Dealer.h>
#include <Tools.h>

//MSE
#include "../include/MSE_Functions.h"
#include "../include/MSE_BTDHTKey.h"


#include <TorrentFile.h>
#include <SHA1.h>


//in protocol define ,the REQUEST_BLOCK_SIZE is 2^15 to 2^17
//16k block have lower speed tested on 07/03/16

//���ֵ����16�ƺ�Ҳûʲô��
#define PENDING_REQUEST_SIZE	(12)

//the peer can request 16 piece data at onetime
#define PEER_REQUEST_SIZE	(24)	

//the connect timeout in second
//#define CONNECT_TIMEOUT		(10)	//�ֲ��ÿɱ䳬ʱ

//the shake hand timeout after connected, not used
#define SHAKE_RESPONSE_TIMEOUT (8)

//the bitset timeout after shaked, not used
#define BITSET_RESPONSE_TIMEOUT (10)

extern void syslog( std::string info );

CBTPeer::CBTPeer(CPeerAdminBase * manager, bool uploadmode)
{

	m_LinkStatus=LS_INIT;
	m_bFastExtension = false;
	m_ActiveCheckTimer = 0;
	m_NewPieceNoticeCheckTimer = 0;

	m_LastPieceData = 0;
	m_LastPeerActiveTick = 0;
	m_nBitSetTimeTick= 0;
	m_nShakeTimeTick=0;  //the time that shake
	m_nConnectTimeTick=0; //the time that connected
	m_ActiveTimerCounter=0;
	m_bGotHashFromPeer = false;
	m_bGotShakeFromPeer = false;
	m_bPeerChokeMe = true;
	m_bMeChokePeer = true;
	m_bPeerInterestMe = false;
	m_bMeInterestPeer = false;
	m_bCanRead = false;
	m_bCanWrite = false;
	m_nDownloadSum=0;
	m_nUploadSum=0;
	m_bAccepted = false;
	m_bGotBitSet=false;
	m_bSendBitSet=false;

	m_bSendShakeToPeer=false;
	m_bEncryption=false; //
	m_bIsA=true;
	m_bGotDHSecret=false;
	m_MSE_pEncryptor=NULL;
	m_MSE_pPeerEncryptor=NULL;
	m_bFullEncryption=false;
	m_bPortExchange=false;
	m_bTransfered=false;
	m_nDownloadPrority=0; //�������ȼ�
	m_nUploadPriority=0;  //�ϴ����ȼ�
	m_MSE_State=MSE_INIT;

	m_pParent = manager;
	m_bUploadMode = uploadmode;
	m_MSE_Drop1K=false;
	m_bSeed=false;
	m_bBrother=false;

	m_nAvDownSpeed=0;
	m_bUtPex=false;
	m_nChokedTick=GetTickCount();

	m_nTestReadBytes=0;
	m_bTestRead=false;
	m_bReportEncryptable=false;
}

CBTPeer::~CBTPeer()
{

	if(m_MSE_pEncryptor!=NULL)
	{
		delete m_MSE_pEncryptor;
	}

	if(m_MSE_pPeerEncryptor!=NULL)
	{
		delete m_MSE_pPeerEncryptor;
	}

	Close();


}


bool CBTPeer::Connect(unsigned int iip, unsigned short iport, unsigned int timeout)
{
	m_PeeriIP = iip;
	m_PeeriPort = iport;
	m_LinkStatus = LS_CONN;
	
//	wchar_t msg[128];
//	swprintf(msg,L"linking ip=%d, timeout=%d",m_PeeriIP,timeout);
//	OutMsg(msg);

	SockLib::TInetAddr4 dest(iip,iport);
	return CSockProxyTCPClientSock::Connect( dest,timeout*1000 );
}



//new, all net byte order
void CBTPeer::Attach(int handle, unsigned int iip , unsigned short iport )
{
	CSockProxyTCPClientSock::Attach( handle );

	m_PeeriIP= iip;
	m_PeeriPort = iport;

	m_bAccepted = true;
	m_bIsA=false;		//are we at A side? only for MSE	

	assert(m_pDealer!=NULL);

	m_nConnectTimeTick=GetTickCount();  //record connect time
	m_LastPeerActiveTick = m_nConnectTimeTick;
	m_LinkStatus = LS_CONNOK;

	maskRead( true );
	maskWrite( false );

	m_sendBuffer.resize( 0 );
	m_recvBuffer.resize( 0 );

	m_bGotShakeFromPeer = false;
	m_bSendShakeToPeer=false;
	m_bMeChokePeer = true;
	m_nChokedTick = GetTickCount();
	m_bMeInterestPeer = false;
	m_bPeerChokeMe = true;
	m_bPeerInterestMe = false;

	m_bCanRead = false;
	m_bCanWrite = true;


	m_MyRequest.Reset(); 

	m_pParent->GetSpeedControl() ->RegisteClient( this );

	assert(m_pDealer!=NULL);

	assert(m_ActiveCheckTimer==0);
	m_ActiveCheckTimer = AddTimer( 60 * 1000 );

	//not good at here
	m_NewPieceNoticeCheckTimer = AddTimer( 3000 );

}

bool CBTPeer::IsAccepted()
{
	return m_bAccepted;
}

//�ⲿ�������رգ����Դ�ԭ��ر�
void CBTPeer::ClosePeer(TCloseReason reason)
{
	m_CloseReason=reason;
	OnClose();
}

void CBTPeer::OnConnectOk()
{

	CSockProxyTCPClientSock::OnConnectOk();
	if(!CanWrite()) return; //not really ok



	m_nConnectTimeTick=GetTickCount();  //record connect time
	m_LastPeerActiveTick = m_nConnectTimeTick;
	m_LinkStatus = LS_CONNOK;

	maskRead( false );
	maskWrite( false );

	m_sendBuffer.resize( 0 );
	m_recvBuffer.resize( 0 );

	m_bGotShakeFromPeer = false;
	m_bSendShakeToPeer=false;
	m_bMeChokePeer = true;
	m_nChokedTick = GetTickCount();
	m_bMeInterestPeer = false;
	m_bPeerChokeMe = true;
	m_bPeerInterestMe = false;

	m_bCanRead = false;

	m_bCanWrite = true;


	m_MyRequest.Reset(); 

	m_pParent->GetSpeedControl() ->RegisteClient( this );

	assert(m_pDealer!=NULL);

	assert(m_ActiveCheckTimer==0);
	m_ActiveCheckTimer = AddTimer( 60 * 1000 );

	//not good at here
	m_NewPieceNoticeCheckTimer = AddTimer( 3000 );

}

void CBTPeer::OnConnectFail()
{
//	wchar_t msg[128];
//	swprintf(msg,L"linkfail ip=%d",m_PeeriIP);
//	OutMsg(msg);

	CSockProxyTCPClientSock::OnConnectFail();
	m_CloseReason=CR_LINKFAIL;
	m_LinkStatus=LS_CLOSE;

}

void CBTPeer::OnConnectionClose()
{
	assert(m_pParent);
	assert(m_pParent->GetSpeedControl());

	m_pParent->GetSpeedControl() ->UnregisteClient( this );

	if(m_bGotBitSet)
	{
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );
	}


	if(!m_MyRequest.Empty())
	{//change for single port listen
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		
		SaveOrphanToStorage(); ////���ﲻ�ȶ�???

		assert(!m_MyRequest.Empty());
		assert(m_MyRequest.GetIndex()>=0);

		m_pParent->GetSession()->GetStorage()->AbandonPieceTask(m_MyRequest.GetIndex());
		m_MyRequest.Reset();

	}

//#ifdef _CHECK
//	wchar_t msg[128];
//	swprintf(msg,L"connection close, peer=%u, reason=%d",m_PeeriIP, m_CloseReason);
//	OutMsg(msg);
//#endif
}

void CBTPeer::OnRead()
{
	CSockProxyTCPClientSock::OnRead();
	if(CanRead())	{
		m_bCanRead = true;
		m_bTestRead=true; //test
	}

}

void CBTPeer::OnWrite()
{
	CSockProxyTCPClientSock::OnWrite();
	if(CanWrite()) {
		m_bCanWrite = true;
	}

}

void CBTPeer::OnClose()
{
	CSockProxyTCPClientSock::OnClose();

	if(m_LinkStatus==LS_CONNOK)
	{
		OnConnectionClose();
	}

	m_LinkStatus=LS_CLOSE;

	m_ActiveCheckTimer = 0;
	m_NewPieceNoticeCheckTimer = 0;

	m_bCanRead = false;
	m_bCanWrite = false;
	m_bPeerInterestMe = false;
	m_bMeInterestPeer = false;

}

int CBTPeer::DoRead( int count  )
{

	if ( !m_bCanRead )
	{
		maskRead(true);
		return 0;
	}


	if( count <=0 )
	{//��Ϊ�����������Բ��������������ֿ��������CPU����
		//��ô�û����������ܼ�ʱ������д��ȥ����Ϊû�м���д����
		m_bCanRead=false; //���������������������Ĵ������´ε���ʱ�ָ�����д����
		maskWrite(false); 
		return 0;
	}

	int readCount = 0;

	char buf[ 4 * 1024 ];

	while ( readCount < count )
	{

		//limit speed
		int readSize = MIN( 4 * 1024, count - readCount );

		int ret = recv( m_hSocket, buf, readSize, 0 );

		if ( ret == 0 )
		{
			m_CloseReason=CR_PEERCLOSE;

			if(!m_bSendShakeToPeer)
			{
				m_CloseReason=CR_NOSHAKE;
			}
			else if( !m_bGotShakeFromPeer)
			{
				m_CloseReason=CR_SHAKE;
			}

			this->OnClose();	//�б������⣡����

			return 0;
		}

		else if ( ret <0 )
		{

#if defined( WIN32)||defined(WINCE)

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno ==EWOULDBLOCK  )
#endif
			{
				m_bCanRead = false;
				break;
			}
			else
			{
				m_bCanRead = false;
				m_CloseReason=CR_NETERR;
#if defined( WIN32)||defined(WINCE)

				if(10054==WSAGetLastError())
				{

					//force closed by peer
					m_CloseReason=CR_PEERCLOSE;

					if(!m_bSendShakeToPeer)
					{
						//OutMsg(L"peer force close me before I send shake");
						m_CloseReason=CR_NOSHAKE;
					}
					else if( !m_bGotShakeFromPeer)
					{
						//OutMsg(L"peer force close me after I send shake");
						m_CloseReason=CR_SHAKE_NETERR;
					}
				}
#endif				
				if(!m_bGotShakeFromPeer  && m_bSendShakeToPeer)
				{//�õ�����ǰ���������رն��ǿ��ɵĹرգ��п�����ISP�Ͽ���������
					m_CloseReason=CR_SHAKE_NETERR;
				}

				OnClose();

				return readCount;
			}

		}
		else
		{//ret > 0


			readCount += ret;

			//���ܹ�����ȫ������m_MSE_State == MSE_FINISH���������ȫ�ӹܽ���
			if(m_bEncryption && m_bFullEncryption && m_MSE_State == MSE_FINISH)
			{
				m_MSE_pEncryptor->decrypt((unsigned char*)buf,ret);
				m_recvBuffer.append( ( const char* ) buf, ret );
			}
			else
			{
				m_recvBuffer.append( ( const char* ) buf, ret );
			}
		}

	}

	ProcessData();

	return readCount;

}

TLinkState CBTPeer::GetLinkState()
{
	return m_LinkStatus;
}


void CBTPeer::OnTimer( unsigned int id )
{
	CSockProxyTCPClientSock::OnTimer(id);

	if ( m_NewPieceNoticeCheckTimer == id )
	{
		//check if have new piece should notice peer
		NoticeNewHavePiece();

		//use this timer to calculate the last 2min speed
		CalculateSpeed();

		return ;
	}


	if ( id == m_ActiveCheckTimer )
	{

		m_ActiveTimerCounter++;

		if(m_ActiveTimerCounter==1)
		{
			if(!m_bEncryption && !IsShaked())
			{
				//һ������ͨ����û�������,
				//����ԭ������ʱû�д���48�ֽڷ��������
				m_CloseReason=CR_SHAKE_TIMEOUT;

				OnClose();
				return;
			}

		}
		else if( m_ActiveTimerCounter == 2 ) 
		{
			if(!m_bEncryption && !m_bGotBitSet )
			{//�Ǽ�������û��2���ڵõ�bitset
				m_CloseReason=CR_BIT_TIMEOUT;
#ifdef _CHECK
//				syslog("bitset timeout(no enc) ");
//				syslog(GetPeerInfo());
//				syslog("\n");
#endif
				OnClose();
				return;
			}

			if(m_bEncryption && !IsShaked())
			{	//��������û��2�������������
				m_CloseReason=CR_SHAKE_TIMEOUT;

				OnClose();
				return;

			}
		}
		else if( m_ActiveTimerCounter == 3 )
		{//��������û��3���ڵõ�bitset
			if(m_bEncryption && !m_bGotBitSet)
			{
				m_CloseReason=CR_BIT_TIMEOUT;
#ifdef _CHECK
//				syslog("bitset timeout(enc) ");
//				syslog(GetPeerInfo());
//				syslog("\n");
#endif
				OnClose();
				return;		
			}
		}

		if(m_MyRequest.TimeOutCheck(60*1000)!=0)
		{//some time we send request when peer choked me, but no error response for non-fast client.
		//it will choke our speed if no resend.
			m_MyRequest.ResetPendingRequest(); //resend it.
		}
	
		unsigned int now = GetTickCount();


		//check the link active
		if ( now - m_LastPeerActiveTick > 2 * 60 * 1000 )
		{
			m_CloseReason=CR_NOT_ACTIVE;
			OnClose();
			return;
		}


		//keep us active
		if ( now - m_LastMyActiveTick > 90 * 1000 )
		{
			//send active packet
			SendKeepAlive();
		}

	}

}

static char sShake[]={
	19,	
	'B','i','t','T','o','r','r','e','n','t',' ','p','r','o','t','o','c','o','l',
	0,0,0,0,0,0,0x01,0x04|0x01,
};

void CBTPeer::MakeShake(char* sbuf)
{


	//sbuf[0]=19;
	//strcpy( sbuf+1, "BitTorrent protocol");
	//memset( sbuf+20, 0, 8);
	//sbuf[27]|=0x04; //fast extension
	//sbuf[27]|=0x01; //DHT port exchange;
	//sbuf[26]|=0x01; //benliud port exchange, this bit maybe conflict with others 
	////ut_pex extension, only support in not accepted.
	////we are initail use this can notice our listen port to peer
	////if(!m_bAccepted) sbuf[25]|=0x10; 
	////sbuf[25]|=0x10;	//ut_pex

	memcpy(sbuf, sShake, 28); //��Χ[0,27]�ֽ�
	memcpy(sbuf+28, m_pParent->GetSession() ->GetTorrentFile() ->GetInfoHash().data(), 20); //[28,47]
	memcpy(sbuf+48, m_pParent->GetSession() ->GetMyID(), 20 ); //[48-67]

}

void CBTPeer::SendHandshake()
{

	char shakebuf[68];
	MakeShake(shakebuf);
	SendData( shakebuf, 68 );
	m_LastMyActiveTick = GetTickCount();
	m_bSendShakeToPeer=true;

}


void CBTPeer::sendInterested( bool interested )
{

	if(m_bMeInterestPeer && interested) return;
	if(!m_bMeInterestPeer && !interested) return;

	char buf[ 5 ];
	//*( ( int* ) buf ) = htonl( 1 );
	long tmp=htonl(1);
	memcpy(buf, &tmp, 4);

	if ( interested )
	{
		//*( ( char* ) ( buf + 4 ) ) = 2;
		buf[4]=2;
		m_bMeInterestPeer = true;
	}
	else
	{
		//*( ( char* ) ( buf + 4 ) ) = 3;
		buf[4]=3;
		m_bMeInterestPeer = false;
	}

	SendData( buf, sizeof( buf ) );

	m_LastMyActiveTick = GetTickCount();

}

void CBTPeer::SendBitfield()
{
	//��Щ�޴��Ŀͻ��˲��ֱܷ����Ƿ���Ķ����ӣ�����ͬһ�������Ϸ�����bitset
	//����������������ͻ�Ҳ�����Σ��Է��ͻỳ�������������ˡ�

	if(m_bSendBitSet) return; 


	CBTPiece bitset;
	bitset= m_pParent->GetSession()->GetBitSet();

	m_NewPieceNoticeListMutex.Lock();
	m_NewPieceNoticeList.clear();
	m_NewPieceNoticeListMutex.Unlock();

	m_LastMyActiveTick = GetTickCount();

	if ( bitset.IsEmpty() && m_bFastExtension )
	{
		SendHaveNone();
		m_bSendBitSet=true;
		return ;
	}
	else if ( bitset.IsAllSet() && m_bFastExtension )
	{
		SendHaveAll();
		m_bSendBitSet=true;
		return ;
	}


	std::string bitfield = bitset.GetStream();
	std::string buf;
	char header[ 5 ];
	//*( ( unsigned int* ) ( header ) ) = htonl( 1 + bitfield.size() );
	long tmp=htonl( 1 + bitfield.size() );
	memcpy(header, &tmp, sizeof(long));

	//*( ( char* ) ( header + 4 ) ) = 5;
	header[4]=5;

	buf.append( header, 5 );
	buf+=bitfield;
	//buf.append( bitfield.data(), bitfield.size() );

	SendData( buf.data(), buf.size() );

	m_bSendBitSet=true;
	m_LastMyActiveTick = GetTickCount();
}

void CBTPeer::SendHave( int index )
{
	char buf[ 9 ];
	//*( ( unsigned int* ) buf ) = htonl( 5 );
	long tmp=htonl(5);
	memcpy(buf, &tmp, 4);
	//*( ( unsigned char* ) ( buf + 4 ) ) = 4;
	buf[4]=4;
	//*( ( int* ) ( buf + 5 ) ) = htonl( index );
	tmp=htonl(index);
	memcpy( buf+5, &tmp, 4);

	SendData( buf, sizeof( buf ) );
	m_LastMyActiveTick = GetTickCount();
}

void CBTPeer::SendRequest( int index, unsigned int offset, unsigned int len )
{

	char buf[ 17 ];

	long tmp;
	//*( ( unsigned int* ) buf ) = htonl( 13 );
	tmp=htonl(13);
	memcpy( buf, &tmp, 4);
	//*( ( unsigned char* ) ( buf + 4 ) ) = 6;
	buf[4]=6;
	//*( ( unsigned int* ) ( buf + 5 ) ) = htonl( (unsigned int)index );
	tmp=htonl((unsigned int)index);
	memcpy(buf+5, &tmp, 4);

	//*( ( unsigned int* ) ( buf + 9 ) ) = htonl( offset );
	tmp=htonl(offset);
	memcpy( buf+9, &tmp, 4);
	//*( ( unsigned int* ) ( buf + 13 ) ) = htonl( len );
	tmp=htonl(len);
	memcpy( buf+13, &tmp, 4);

	SendData( buf, 17 );

	m_LastMyActiveTick = GetTickCount();


}

void CBTPeer::SendPieceData( int index, unsigned int offset, std::string& data )
{
	char buf[ 13 ];
	//*( ( unsigned int* ) buf ) = htonl( 9 + data.size() );
	long tmp;
	tmp=htonl( 9 + data.size() );
	memcpy(buf, &tmp, 4);

	//*( ( unsigned char* ) ( buf + 4 ) ) = 7;
	buf[4]=7;

	//*( ( unsigned int* ) ( buf + 5 ) ) = htonl( (unsigned int)index );
	tmp=htonl( (unsigned int)index );
	memcpy(buf+5, &tmp, 4);

	//*( ( unsigned int* ) ( buf + 9 ) ) = htonl( offset );
	tmp=htonl(offset);
	memcpy(buf+9, &tmp, 4);

	std::string pkg;
	pkg.append( ( const char* ) buf, sizeof( buf ) );
	pkg.append( data.c_str(), data.size() );

	SendData( pkg.data(), pkg.size() );

	m_LastMyActiveTick = GetTickCount();

}

void CBTPeer::SendPieceCancel( int index, unsigned int offset, unsigned int len )
{
	char buf[ 17 ];

	//*( ( unsigned int* ) buf ) = htonl( 13 );
	long tmp=htonl(13);
	memcpy(buf, &tmp, 4);
	//*( ( unsigned char* ) ( buf + 4 ) ) = 8;
	buf[4]=8;
	//*( ( unsigned int* ) ( buf + 5 ) ) = htonl( (unsigned int) index );
	tmp=htonl((unsigned int) index );
	memcpy(buf+5, &tmp, 4);
	//*( ( unsigned int* ) ( buf + 9 ) ) = htonl( offset );
	tmp=htonl(offset);
	memcpy(buf+9, &tmp, 4);

	//*( ( unsigned int* ) ( buf + 13 ) ) = htonl( len );
	tmp=htonl(len);
	memcpy(buf+13, &tmp, 4);

	SendData( buf, sizeof( buf ) );
	m_LastMyActiveTick = GetTickCount();
}

//���ܹ�����ȫ������m_MSE_State==MSE_FINISH���˺����Žӹܼ���
void CBTPeer::SendData( const void* data, size_t len )
{
#define MAX_MSGLEN  (32*1024) //see RC4Encryptor


	const unsigned char* cdata=(const unsigned char*)data;

	if(m_bEncryption && m_bFullEncryption  && m_MSE_State==MSE_FINISH )
	{

		if(len > MAX_MSGLEN)
		{
			//split to slice and encrypt it
			for(unsigned int i=0; i< len/MAX_MSGLEN ; i++)
			{
				m_sendBuffer.append( (const char*)(m_MSE_pEncryptor->encrypt( cdata+i*MAX_MSGLEN , MAX_MSGLEN )), MAX_MSGLEN );
			}

			//then the left msg
			if(len % MAX_MSGLEN )
			{
				m_sendBuffer.append( (const char*)(m_MSE_pEncryptor->encrypt( cdata + (len - len%MAX_MSGLEN) , len%MAX_MSGLEN) ), len%MAX_MSGLEN);
			}
		}
		else
		{
			m_sendBuffer.append( (const char*)(m_MSE_pEncryptor->encrypt( cdata , len )), len );
		}

	}
	else
	{
		m_sendBuffer.append( ( const char* ) data, len );
	}

	maskWrite(true); //�Ƿ���Ҫ��Ҫ����
}

int CBTPeer::ProcessData()
{
	//�������ӵļ��ܴ�������
	if(!m_bAccepted && m_bEncryption && m_MSE_State!=MSE_FINISH)
	{//we are in DH secret progress block it to new process
		//��������������������Ҽ�������û����
		//assert(m_bIsA);
		if(!DoDHSecretShake()) //if DoDHSecretShake return false, no continue do the later
		{
			return 0;
		}

	}

	//�������ӵļ��ܴ�������
	if(m_bAccepted && !m_bGotShakeFromPeer)
	{
		assert(!m_bIsA);
		//accepted peer we don't send shake first,so this packet should be the shake of peer
		if(!m_bEncryption) //ȱʡ�ǲ����ܵ�
		{
			if(!CheckAcceptedShakeHand()) //������ܻ����ü���״̬m_bEncryption
			{
				return 0;
			}

		}
		else
		{//�Ѿ���CheckAcceptedShakeHandȷ�Ͻ����˼���״̬���������ڵļ��ܽ��ȣ����ü��ܴ�������
			//��������ֻ���ܼ������Ӷ������˼���λm_bEncryption��������
			switch(m_MSE_State)
			{
			case MSE_INIT: //�Ѿ�ȷ���˼���m_bEncryption������û�õ�pubkey
				MSE_AfterConfirmEncrypt();
				//break;	//bug, cause CheckHandshake assert fail.
				return 0;
			case MSE_SEND_PUB:
				if(!MSE_AfterSendPub()) return 0;
				break;
			case MSE_FOUND_REQ1:

				if(!MSE_AfterFoundReq1()) return 0;
				break;
			case MSE_WAIT_PAD_C:
				if(!MSE_AfterWaitPadC()) return 0;
				break;
			case MSE_WAIT_IA:
				if(!MSE_AfterWaitIA()) return 0;
				break;
			case MSE_FINISH: //nothing to do, make gcc happy
			case MSE_GOT_PUB:
			case MSE_FOUND_VC:
			case MSE_WAIT_PAD_D:

				break;
			}
		}
	}

	//���ְ����
	if ( !m_bGotShakeFromPeer )
	{

		if ( m_recvBuffer.size() >= 68 )
		{

			std::string shake = m_recvBuffer.substr( 0, 68 );
			m_recvBuffer.erase( 0, 68 );

			if ( !CheckHandshake( shake ) )
			{
				return 0;
			}

			m_LastPeerActiveTick = GetTickCount();


			//���Լ�������ִ�а�����

			if(m_bReportEncryptable)
			{
				m_pParent->GetSession()->PeerSupportEncryption(m_PeeriIP,m_bEncryption);
			}

		}
		else
		{
			return 0;
		}

	}


	//������϶��ǻ�������ְ�
	while ( m_recvBuffer.size() >= 4 )
	{

		//unsigned int packlen = *( ( int* ) m_recvBuffer.data() );
		unsigned int packlen;
		memcpy( &packlen, m_recvBuffer.data(), sizeof(unsigned int));

		packlen = ntohl( packlen );

		if ( packlen == 0 )
		{
			m_LastPeerActiveTick = GetTickCount();
			m_recvBuffer.erase( 0, 4 );
			continue;
		}

		//���ĳ���У�飬̫��̫�̶����ܲ���
		// len ��С����==1
		if( packlen > 4 + 8 + _CHUNK_LEN )
		{
			m_CloseReason=CR_BAD_DATA;
			OnClose();
			return 0;
		}

		if ( m_recvBuffer.size() >= ( packlen + 4 ) )
		{//��������

			unsigned char cmdID = m_recvBuffer[ 4 ];

			int nret= DoCmd( cmdID, ( void* ) ( m_recvBuffer.data() + 5 ), packlen - 1 );

			m_recvBuffer.erase( 0, packlen + 4 );
		}
		else
		{
			break;
		}
	}


	return 0;
}

//����������������м������
//1���Է�����������ӣ������׶ε����������У��HASH�������������ת�ƣ�����ʱ�Ѿ�ת������
//2���Է�������ͨ���Ӳ�����������
//3���ҷ�������ͨ���ӣ��Է�����ʱ���������飬��ת������
//4���ҷ�����������ӣ������������飬��ת�����ӡ�
//�������������Ӧ�ò���ת������
bool CBTPeer::CheckHandshake( std::string info )
{

	assert(m_pParent->GetSession()!=NULL); //��������LINUX��������


	char first=info[0];

	if ( first != 19 )
	{
		m_CloseReason=CR_PROTOCOL;
#ifdef _CHECK
		//OutMsg(L"shake check first!=19 fail",MSG_ERROR);
#endif
		OnClose();

		return false;
	}

	std::string str = info.substr( 1, 19 );

#ifdef WIN32
	if ( _stricmp( str.c_str(), "BitTorrent protocol" ) != 0 )
#else
	if ( strcasecmp( str.c_str(), "BitTorrent protocol" ) != 0 )
#endif
	{
		m_CloseReason=CR_PROTOCOL;
#ifdef _CHECK
		//OutMsg(L"close1");
#endif
		OnClose();
		return false;
	}

	if ( info[ 27 ] & 0x04 )
	{
		m_bFastExtension = true;  //fast extension enabled!
	}

	if( info[26] & 0x01)
	{
		m_bPortExchange=true;
	}

	if( info[25] & 0x10)
	{
		m_bUtPex=true;  //peer support ut_pex
	}

	std::string peerhash = info.substr( 28, 20 );

	//if ( memcmp( peerhash.data(), myhash.data(), 20 ) != 0 )
	if(peerhash!=m_pParent->GetSession() ->GetTorrentFile() ->GetInfoHash())
	{
		m_CloseReason=CR_BAD_DATA;
#ifdef _CHECK
		//OutMsg(L"bad infohash close");
#endif
		OnClose();
		return false;
	}


	m_PeerId = info.substr( 48, 20 );

	m_bGotShakeFromPeer = true;
	m_nShakeTimeTick=GetTickCount(); //record the shake time


	//check if this peer is myself

	if(m_pParent->GetSession()->IsSelfPeerId(m_PeerId))
	{
		m_CloseReason=CR_SELF;

		OnClose();

		return false;

	}

	CheckAgent(); //check on m_PeerId

	if(m_bAccepted)
	{
		assert(!m_bIsA);
		assert(m_bTransfered);

		if(!m_bSendShakeToPeer)	
		{
			SendHandshake();
		}

	}
	else
	{
		//
		SendBitfield();
		SendListenPort();
	}


	m_LastMyActiveTick = GetTickCount();
	m_LastPeerActiveTick = m_LastMyActiveTick;

	return true;
}

int CBTPeer::DoCmd( unsigned char cmd, void * data, size_t dataLen )
{

	switch ( cmd )
	{

	case 0x00:
		return DoCmdChoke( data, dataLen );
	case 0x01:
		return DoCmdUnchoke( data, dataLen );
	case 0x02:
		return DoCmdInterested( data, dataLen );
	case 0x03:
		return DoCmdNotInterested( data, dataLen );
	case 0x04:
		return DoCmdHave( data, dataLen );
	case 0x05:
		return DoCmdBitfield( data, dataLen );
	case 0x06:
		return DoCmdRequest( data, dataLen );
	case 0x07:
		return DoCmdPiece( data, dataLen );
	case 0x08:
		return DoCmdCancel( data, dataLen );
		//DHT extension
	case 0x09:
		return DoCmdDHTPort( data ,dataLen );
		//fast extension
	case 0x0D:
		return DoCmdSuggestPiece( data, dataLen );
	case 0x0E:
		return DoCmdHaveAll( data, dataLen );
	case 0x0F:
		return DoCmdHaveNone( data, dataLen );
	case 0x10:
		return DoCmdRejectRequest( data, dataLen );
	case 0x11:
		return DoCmdAllowFast( data, dataLen );
	case 0x14://ut_pex extertion
		return DoPexCommand(data,dataLen);
	case 0xE0: //������չ����Է��ļ����˿�
		return	DoCmdPort( data, dataLen); 
	default:
		{
			/*
#ifdef _CHECK
			wchar_t msg[128];
			swprintf(msg,L"unknown cmd=%d, len=%d", cmd, dataLen);
			OutMsg(msg,MSG_ERROR);
			PrintPeerInfo();
#endif
			*/
			return -1;
		}

		break;
	}

	return 0;
}

int CBTPeer::DoCmdChoke( void * data, size_t dataLen )
{
	if(!IsShaked())
	{
		//		OutMsg(L"choke before shake", MSG_ERROR);
		return -1;
	}

	if ( dataLen != 0 )
	{
		return -1;
	}

	m_bPeerChokeMe = true;
	m_nChokedTick = GetTickCount();


	m_MyRequest.Choke(true);

	//in fast extention, choke doesn't means clear the request, so don't need resend the request
	if ( !m_bFastExtension )
	{
		m_MyRequest.ResetPendingRequest();
	}

	m_LastPeerActiveTick = GetTickCount();

#ifdef _CHECK	
	//at least, choke give me some infomation that I am not so good for peer
	//wchar_t msg[128];
	//swprintf(msg,L"choked by %u",m_PeeriIP);
	//OutMsg(msg);
#endif
	return 0;
}

int CBTPeer::DoCmdUnchoke( void * data, size_t dataLen )
{
	if (!IsShaked()) 
	{
		return -1;
	}

	if ( dataLen != 0 )
	{
		return -1;
	}

	m_MyRequest.Choke(false);

	m_bPeerChokeMe = false;

	m_LastPeerActiveTick = GetTickCount();
#ifdef _CHECK	
	//wchar_t msg[128];
	//swprintf(msg,L"unchoke by %u",m_PeeriIP);
	//OutMsg(msg);
#endif
	return 0;
}

int CBTPeer::DoCmdInterested( void * data, size_t dataLen )
{
	if (!IsShaked()) 
	{
		return -1;
	}
	if ( dataLen != 0 )
	{
		return -1;
	}

	m_bPeerInterestMe = true;

	m_LastPeerActiveTick = GetTickCount();

	return 0;
}

int CBTPeer::DoCmdNotInterested( void * data, size_t dataLen )
{
	if (!IsShaked()) 
	{
		return -1;
	}
	if ( dataLen != 0 )
	{
		return -1;
	}

	m_bPeerInterestMe = false;

	if ( m_bUploadMode )
	{//we are in upload mode ,but peer don't interest me, close it
		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return 0;
	}

	m_LastPeerActiveTick = GetTickCount();

	m_PeerRequestList.clear();
	return 0;
}

int CBTPeer::DoCmdHave( void * data, size_t dataLen )
{


	if ( dataLen != 4 )
	{

		return -1;
	}

	if(!IsShaked()) 
	{

		return -1; //����m_pParent->GetSession()
	}

	//int index = *( ( int* ) data );
	int index;
	memcpy(&index, data, sizeof(int));

	index = ntohl( index );


	if(index <0 ||index >= int(m_pParent->GetSession()->GetStorage()->GetPieceCount()))
	{

		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return -1;
	}


	if(!m_bGotBitSet) 
	{

		//������ǵõ���bitset. �Ժ����ظ��õ���ȡ���ȵõ����ĸ�bitset

		m_PeerBitSet.Init(m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount() );
		m_PeerBitSet.Set(index, true);

		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, true );
		m_bGotBitSet=true;

		SendBitfield(); // ppc �������
	}
	else
	{

		//�п������Ƭ�Ѿ����ڣ�������Ҫȷ�����ظ��������ύ����ظ��Ķ�����
		if(m_PeerBitSet.IsSet(index))
		{
			//�ظ���Ƭ���Ѿ��������ˣ������������ٴ��ύ���������ĵ�ͳ�ƾͲ�һ����
			return 0; //�����ں����жϣ���Ϊ�ޱ仯
		}
		else
		{

			m_PeerBitSet.Set( index, true );
			//notice manager piece got
			assert(m_pParent->GetSession()!=NULL);
			assert(m_pParent->GetSession()->GetStorage()!=NULL);

			m_pParent->GetSession()->GetStorage() ->PeerHaveNewPieceNotice(m_PeeriIP,index);
		}
	}


	m_nBitSetTimeTick=GetTickCount();

	if(m_PeerBitSet.IsAllSet()) m_bSeed=true;

	m_LastPeerActiveTick = GetTickCount();
	//add for new way request
	//check if we interest this piece, if yes,send interest

	if (!m_bMeInterestPeer && m_pParent->GetSession() ->GetStorage() ->IsPieceInterest( index ) )
	{

		sendInterested( true );
	}

	return 0;
}

int CBTPeer::DoCmdBitfield( void * data, size_t dataLen )
{

	if(!IsShaked()) return 0; //m_pParent->GetSession()��û�м�飬���û�з���ת��ǰ�ͳ��������������

	int piecenum=m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount();
	int bytenum= (piecenum/8) + ((piecenum%8)?1:0);

	if(dataLen != (unsigned int)bytenum) {
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}


	if(m_bGotBitSet)
	{//�ظ���bitset,����

#ifdef _CHECK
		//OutMsg(L"got bitset again!!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		//ȡ���ϵ�bitset,��������һ��
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );

	}

	std::string bitset;
	bitset.append( ( const char* ) data, (unsigned int)bytenum);
	m_PeerBitSet.Init( bitset, piecenum );
	m_bGotBitSet=true;	

	m_nBitSetTimeTick=GetTickCount();

	//������bitset�Ƿ�����α��Ļ��˺ۼ�
	int diff=m_pParent->GetSession()->CheckBitSet(m_PeerId,m_PeeriIP,m_PeerBitSet);
	if(diff<0)
	{
		if(m_bUploadMode)
		{
#ifdef _CHECK
			//OutMsg(L"close for honest",MSG_WARNNING);
#endif
			m_CloseReason=CR_HONEST;
			OnClose();
			return 0;
		}

		/*
		else
		{
#ifdef _CHECK
			OutMsg(L"decrease credit for honest",MSG_WARNNING);
			PrintPeerInfo();
			printf("diff=%d\n",diff);
#endif
			m_nPeerCredit--;
		}
		*/
	}

	if(m_PeerBitSet.IsAllSet()) m_bSeed=true;

	if(m_bAccepted)
	{
		//�ǽ��ܽ������ӣ������Ȳ���BITSET�����յ��Է��ĺ��ٷ�
		assert(!m_bIsA);
		SendBitfield();
		SendListenPort();

	}

	if(m_bUploadMode && !m_pParent->GetSession()->GetStorage()->IsPeerNeedMyPiece(m_PeerBitSet))
	{//we are in upload mode ,only pick the peer that need my data

		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return 0;

	}

	//notice manager piece got
	//m_pParent->PieceChange( m_PeerBitSet, true );
	m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, true );

	m_LastPeerActiveTick = GetTickCount();

	if(!m_bUploadMode)
	{//only not in upload mode need check it
		//check if we interest it's data
		//if yes, send interest
		//IsPieceInterestӦ�÷������Ǹ���Ȥ��Ƭ�����������������ۿɽ������Ĳ�����2007/09/07
		//��������ѡ��������ʱ�����Ը�׼ȷ���ж϶Է����ݶ����ǵļ�ֵ
		if ( m_pParent->GetSession() ->GetStorage() ->IsPieceInterest( m_PeerBitSet ) )
		{
			sendInterested( true );
		}
		else
		{
			//maybe we don't need data anymore
			sendInterested(false);
		}
	}
	else
	{//we in upload mode ,not interest
		sendInterested(false);

	}

	if(m_bFastExtension && m_PeerBitSet.GetSetedCount()<20)
	{//maybe we can help it with allow fast piece
		GenAllowFastPieceList();
	}

	return 0;
}

int CBTPeer::DoCmdRequest( void * data, size_t dataLen )
{
	if (!IsShaked()) //����m_pParent->GetSession()
	{
		return -1;
	}

	if ( dataLen != 12 )
	{
		return -1;
	}

	if( !m_bSendBitSet)
	{
#ifdef _CHECK
		//OutMsg(L"got request but we don't send bitset!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		return -1;
	}


	//get all the data we need	
	//int index = *( ( int* ) data );
	int index;
	memcpy(&index, data, sizeof(int));
	index = ntohl( index );
	//unsigned int offset = *( ( unsigned int* ) ( ( char* ) data + 4 ) );
	unsigned int offset;
	memcpy(&offset, (char*)data+4, sizeof(unsigned int));

	offset = ntohl( offset );
	//unsigned int len = *( ( unsigned int* ) ( ( char* ) data + 8 ) );
	unsigned int len;
	memcpy(&len, (char*)data+8, sizeof(unsigned int));
	len = ntohl( len );


	//set last active mark
	m_LastPeerActiveTick = GetTickCount();

	//��������Ƿ�Խ��
	if(index <0 ||index >= int(m_pParent->GetSession() ->GetStorage()->GetPieceCount()))
	{
#ifdef _CHECK
		//OutMsg(L"request index out of range!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}

	if ( m_bFastExtension && IsMeAllowFastPiece(index))
	{

		//check if the piece in allow fast list, if yes, send the data right now and don't care the choke
		//�������allowfast piece ,��������

		std::string pieceDdata;

		if ( m_pParent->GetSession() ->GetStorage() ->ReadData( pieceDdata, index, offset, len ) &&
			pieceDdata.size() == len )
		{
			SendPieceData( index, offset, pieceDdata );
	
			//report to storage
			m_pParent->GetSession()->SumUpload(m_PeeriIP,len);
		}

		//set last active mark
		m_LastPeerActiveTick = GetTickCount();

		return 0; //�Ѿ��������ݣ����ؼ�¼�����ˣ�����


	}

	//is that seed? don't accept seed's request
	if(m_bSeed) {
#ifdef _CHECK
		//OutMsg(L"got seed request ",MSG_ERROR);
		//PrintPeerInfo();
#endif
		return 0;
	}

	//only fast extension in choke accept request
	if( !m_bFastExtension && m_bMeChokePeer) {
#ifdef _CHECK
		//OutMsg(L"got request when choked",MSG_ERROR);
		//PrintPeerInfo();
#endif
		return 0;
	}

//allow fast piece have processed,here not allowfast piece.
//should reject it when we choked peer
	if( m_bFastExtension && m_bMeChokePeer )
	{
		SendRejectRequest( index, offset, len );
		return 0;
	}


	//check if duplicate request
	TPeerPieceRequestList::iterator it;

	for ( it = m_PeerRequestList.begin(); it != m_PeerRequestList.end(); it++ )
	{
		if ( it->index == index && it->offset == offset )
		{
			return 0;
		}
	}

	//check if too many pending request, we only allow PEER_REQUEST_SIZE pending request
	if ( m_PeerRequestList.size() >= PEER_REQUEST_SIZE )
	{
		//too many request, reject the request
		SendRejectRequest( index, offset, len );

		return 0;
	}

	//check if we have this piece already?
	if ( !m_pParent->GetSession() ->GetStorage() ->IsFinishedPiece( index ) )
	{
		SendRejectRequest( index, offset, len );
		return 0;
	}


	//add new request to list ,
	TPeerBlockRequest req;

	req.index = index;

	req.offset = offset;

	req.len = len;

	m_PeerRequestList.push_back( req );

	//OutputDebugString(L"got a request!\n");

	//UT��Ѹ�׶��ܸߣ��ﵽ20��
/*
#ifdef _CHECK
	if(m_PeerRequestList.size()>8)
	{
		wchar_t msg[128];
		swprintf(msg,L"request size =%d",m_PeerRequestList.size());
		OutMsg(msg);
		PrintPeerInfo();
	}
#endif
	*/

	return 0;
}


int CBTPeer::DoCmdPiece( void * data, size_t dataLen )
{

	if (!IsShaked()) 
	{
		return -1;
	}

	if ( dataLen <= 8 )
	{
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return -1;
	}

	//int index = *( ( int* ) data );
	int index;
	memcpy(&index, data, sizeof(int));
	index = ntohl( index );
	//unsigned int offset = *( ( unsigned int* ) ( ( char* ) data + 4 ) );
	unsigned int offset;
	memcpy(&offset, (char*)data+4, sizeof(unsigned int));

	offset = ntohl( offset );

	unsigned int len = dataLen - 8;


	if(index<0||index >= int(m_pParent->GetSession() ->GetStorage()->GetPieceCount()))
	{
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}

	if(m_pParent->GetSession()->GetStorage()->IsPieceFinish(index))
	{
		m_LastPeerActiveTick = GetTickCount();

		return 0;
	}

	std::string block;
	block.append( ( const char* ) data + 8, len );

	if(index!=m_MyRequest.GetIndex()) 
	{
		//wrong index not my request!

		m_nDownloadSum+= len/4096;
		
		//put the data to storage as an orphan
		m_pParent->GetSession() ->GetStorage() ->CheckInOrphanData( index, m_PeeriIP, offset, block );
		
		m_pParent->GetSession()->SumDownload(m_PeeriIP, len );
		
		m_LastPieceData = GetTickCount();
		m_LastPeerActiveTick = m_LastPieceData;
		
		return 0;

	}

	if(!m_MyRequest.SetData(offset, block)) 
	{ //bad data ,close it

		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}


	m_nDownloadSum += len/4096;

	m_pParent->GetSession() ->SumDownload( m_PeeriIP, len );

	m_LastPieceData = GetTickCount();

	m_LastPeerActiveTick = m_LastPieceData;

	//submit to storage if this task is coorperate task

	if(m_MyRequest.IsCoorperate()) 
	{

		if(!m_pParent->GetSession()->GetStorage()->SubmitShareData(m_PeeriIP,index,offset,block))
		{
			//no need to download any more, it's finished!
			CancelMyRequest();
			return 0;
		}

	}
//#ifdef _CHECK	
//	wchar_t msg[128];
//	swprintf(msg,L"get piece from %u",m_PeeriIP);
//	OutMsg(msg);
//#endif

	//����Ƿ��Ѿ��������

	if(!m_MyRequest.IsFinish()) return 0; 

	std::string pieceData = m_MyRequest.GetPieceData(); // = _pieceRequest.getPiece();


	if( Tools::SHA1String ( pieceData )
		== m_pParent->GetSession()->GetStorage() ->GetPieceHash( index ) )
	{
		m_pParent->GetSession() ->GetStorage() ->WritePiece( index, pieceData );
		CancelMyRequest();//??
		//return 0;
	}
	else
	{
		//if have origin data , abandon the origin data and retry
		//OutMsg(L"checksum fail",MSG_ERROR);
		if(m_MyRequest.HaveAlien(m_PeeriIP)) 
		{
			m_MyRequest.ClearAlien(m_PeeriIP);
		}
		else
		{
			m_pParent->GetSession()->GetStorage()->AbandonPieceTask(m_MyRequest.GetIndex());
			m_MyRequest.Reset(); 

			m_CloseReason=CR_BAD_DATA;
			OnClose();
			//return 0;
		}
	}

	return 0;
}

int CBTPeer::DoCmdCancel( void * data, size_t dataLen )
{
	if ( dataLen != 12 )
	{
		return -1;
	}

	//int index = *( ( int* ) data );
	int index;
	memcpy(&index, data, sizeof(int));

	index = ntohl( index );
	//unsigned int offset = *( ( unsigned int* ) ( ( char* ) data + 4 ) );
	unsigned int offset;
	memcpy( &offset, (char*)data+4, sizeof(unsigned int));

	offset = ntohl( offset );
	//unsigned int len = *( ( unsigned int* ) ( ( char* ) data + 8 ) );
	unsigned int len;
	memcpy(&len, (char*)data+8, sizeof(unsigned int));
	len = ntohl( len );

	TPeerPieceRequestList::iterator it;

	for ( it = m_PeerRequestList.begin();it != m_PeerRequestList.end();it++ )
	{
		if ( it->index == index && it->offset == offset )
		{
			m_PeerRequestList.erase( it ); 
			break;
		}
	}


	m_LastPeerActiveTick = GetTickCount();
	return 0;
}

//should lock the buffer write, because it be called by other thread!
void CBTPeer::BroadcastNewPiece( int index )
{
	//put it to a list , this may be called by other thread,

	if ( !m_bSendBitSet ) //��֤�ȷ���bitset, ��have
	{
		return ;
	}

	SockLib::CAutoLock al(m_NewPieceNoticeListMutex);

	m_NewPieceNoticeList.push_back( index ); 

}

void CBTPeer::CancelMyRequest( int index )
{
	if(m_MyRequest.GetIndex()== index) 
	{
		CancelMyRequest();
	}
}

void CBTPeer::CancelMyRequest()
{

	if(m_MyRequest.Empty()) return;

	int index=m_MyRequest.GetIndex();

	unsigned int offset[PENDING_REQUEST_SIZE*2];
	unsigned int length[PENDING_REQUEST_SIZE*2];

	int count=m_MyRequest.GetPendingRequest(PENDING_REQUEST_SIZE*2,offset,length);

	for(int i=0;i<count;i++)
	{
		SendPieceCancel(index,offset[i],length[i]);
	}

	m_pParent->GetSession() ->GetStorage() ->AbandonPieceTask( m_MyRequest.GetIndex() );

	m_MyRequest.Reset();
}


void CBTPeer::DoMyRequest()
{
	//check the condition in upper function,here don't check anything but send request!
	//do request

	//old type, not coorperate task

	if(!m_MyRequest.IsCoorperate()) 
	{

		while (m_MyRequest.GetPendingCount() < PENDING_REQUEST_SIZE ) 
		{
			unsigned int offset, len;

			if(!m_MyRequest.GetTask(offset,len))
			{
				break;
			}
			else
			{
				SendRequest( m_MyRequest.GetIndex(), offset, len);
			}
		}

	}
	else
	{
		while ( m_MyRequest.GetPendingCount() < PENDING_REQUEST_SIZE ) 
		{

			unsigned int offset,len;


			if(!m_pParent->GetSession()->GetStorage()->GetShareTask(m_MyRequest.GetIndex(),offset,len)) 
			{
				//OutMsg(L"can't get task block-1",MSG_INFO);

				if(m_MyRequest.GetPendingCount()==0)
				{
					//OutMsg(L"no share task to do ,close link");

					m_CloseReason=CR_MYCLOSE;
					OnClose();
					return;
				}
				else
				{
					break;
				}
			}


			if(!m_MyRequest.IsPendingRequest(offset)) 
			{

				if(!m_MyRequest.MarkPendingRequest(offset))
				{
					//OutMsg(L"some wrong-1",MSG_ERROR);
					return;
				}
				else
				{
					//pending marked , send the request
					SendRequest(m_MyRequest.GetIndex(),offset,len);


					if(m_MyRequest.GetPendingCount() < PENDING_REQUEST_SIZE)
					{

						continue;
					}
					else
					{
						//get the block fulfilled the PENDING_REQUEST_SIZE, no more block
						return ;
					}
				}

			}
			else
			{
				//the block get from center can't fulfiled all that i want size(PENDING_REQUEST_SIZE)
				//so we fill the size myself
				//alread my pending request, so ,maybe no block task can be fetch at share
				//just do us request with share request

				while (m_MyRequest.GetPendingCount() < PENDING_REQUEST_SIZE )
				{
					unsigned int offset, len;

					if(!m_MyRequest.GetTask(offset,len))
					{
						break;
					}

					SendRequest(m_MyRequest.GetIndex(), offset,len);
				}

				return;
			}

		}

	}

}

bool CBTPeer::MeChokePeer()
{
	return m_bMeChokePeer;
}
bool CBTPeer::PeerChokeMe()
{
	return m_bPeerChokeMe;
}
bool CBTPeer::PeerInterestMe()
{
	return m_bPeerInterestMe;
}
bool CBTPeer::MeInterestPeer()
{
	return m_bMeInterestPeer;
}

//call by admin
void CBTPeer::ChokePeer( bool choke )
{
	/*
	#ifdef _CHECK
	if(choke)
	{
	wchar_t msg[128];
	swprintf(msg,L"choke peer %u",m_PeeriIP);
	OutMsg(msg);
	}
	else
	{
	wchar_t msg[128];
	swprintf(msg,L"unchoke peer %u",m_PeeriIP);
	OutMsg(msg);
	}
	#endif
	*/
	if ( choke && !m_bMeChokePeer)
	{
		SendChoke();
	}
	else if( !choke && m_bMeChokePeer)
	{
		SendUnChoke();
	}

}


void CBTPeer::DownloadFinish(bool comp)
{
	if(comp)
	{
		m_bUploadMode=true;

		if(!m_bGotBitSet) return;

		sendInterested( false );


		//cancel the request
		//CancelMyRequest();

		//maybe need to close the link

		//����жϲ���׼�ģ���Ϊ�㲻֪���Է��Ƿ���ѡ�����أ����ԣ�����Է�������Ȥ���Ǿ͹ر�
		//if(m_pParent->GetSession()->GetStorage()->IsPeerNeedMyPiece(m_PeerBitSet))
		if(!m_bPeerInterestMe)
		{
			m_CloseReason=CR_MYCLOSE;
			OnClose(); 
			return;
		}

	}
	else
	{
		m_bUploadMode=false;

		if(!m_bGotBitSet) return;

		unsigned int interest= m_pParent->GetSession() ->GetStorage() ->IsPieceInterest( m_PeerBitSet );

		sendInterested( interest!=0 );

	}

}

int CBTPeer::DoCmdSuggestPiece( void * data, int dataLen )
{

	if (!IsShaked()) 
	{
		return -1;
	}

	if ( !m_bFastExtension )
	{
		m_CloseReason=CR_PROTOCOL;
		OnClose();
		return -1;
	}

	//Suggest Piece: <len=0x0005><op=0x0D><index>
	if ( dataLen != 4 )
	{
		return -1;
	}

	//   m_nUploadPriority += 1;

	//int index = ( *( int* ) data );
	int index;
	memcpy(&index, data, sizeof(int));

	index = ntohl( index );

	//��������Ƿ�Խ��
	if(index<0||index >= int(m_pParent->GetSession() ->GetStorage()->GetPieceCount()))
	{
#ifdef _CHECK
		///OutMsg(L"suggest index out of range!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}

	if(m_SuggestList.size() < 20)
	{
		m_SuggestList.push_back( index );
	}

	m_LastPeerActiveTick = GetTickCount();

	return 0;

}

int CBTPeer::DoCmdHaveAll( void * data, int dataLen )
{

	if ( !m_bFastExtension )
	{
		m_CloseReason=CR_PROTOCOL;
		//OutMsg(L"haveall cmd in no fastext",MSG_INFO);
		OnClose();
		return 0;
	}

	if ( dataLen != 0 )
	{
		//OutMsg(L"wrong haveall cmd",MSG_INFO);
		return -1;
	}

	if(m_bUploadMode)
	{//we are in upload mode ,only pick the peer that need my data
		m_CloseReason=CR_MYCLOSE;
		//OutMsg(L"close peer that don't need us");
		OnClose();
		return 0;
	}


	if(m_bGotBitSet)
	{
#ifdef _CHECK
		//OutMsg(L"got bitset again!!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		//ȡ���ϵ�bitset��¼������һ����
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );
	}


	//make the bitfield and notice upper
	m_PeerBitSet.Init( m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount() );
	m_PeerBitSet.SetAll();
	m_bGotBitSet=true;
	m_nBitSetTimeTick=GetTickCount();

	int diff=m_pParent->GetSession()->CheckBitSet(m_PeerId,m_PeeriIP,m_PeerBitSet);

	m_bSeed=true;

	if(m_bAccepted)
	{
		//�ǽ��ܽ������ӣ������Ȳ���BITSET�����յ��Է��ĺ��ٷ�
		assert(!m_bIsA);
		SendBitfield();
		SendListenPort();
	}

	//notice manager piece got
	m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, true );

	m_LastPeerActiveTick = GetTickCount();

	if ( !m_bUploadMode )
	{
		sendInterested( true );
	}

	return 0;
}

int CBTPeer::DoCmdHaveNone( void * data, int dataLen )
{

	if ( !m_bFastExtension )
	{
		m_CloseReason=CR_PROTOCOL;
		OnClose();
		return 0;
	}

	if ( dataLen != 0 )
	{
		return -1;
	}


	if(m_bGotBitSet)
	{
#ifdef _CHECK
		//OutMsg(L"got bitset again!!",MSG_ERROR);
		//PrintPeerInfo();
#endif	
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		//ȡ���ϵ�bitset��¼������һ����
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );

	}

	//no need to notice piece
	m_PeerBitSet.Init( m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount() );
	m_bGotBitSet=true;
	m_nBitSetTimeTick=GetTickCount();

	//������bitset�Ƿ�����α��Ļ��˺ۼ�
	int diff=m_pParent->GetSession()->CheckBitSet(m_PeerId,m_PeeriIP,m_PeerBitSet);
	if(diff<0)
	{
		if(m_bUploadMode)
		{
#ifdef _CHECK
			//OutMsg(L"close for honest");
#endif
			m_CloseReason=CR_HONEST;
			OnClose();
			return 0;
		}
		/*
		else
		{
#ifdef _CHECK
			OutMsg(L"decrease credit for honest(havenone)",MSG_WARNNING);
#endif
			m_nPeerCredit--;
		}
		*/
	}

	if(m_bAccepted)
	{
		//�ǽ��ܽ������ӣ������Ȳ���BITSET�����յ��Է��ĺ��ٷ�
		assert(!m_bIsA);
		SendBitfield();
		SendListenPort();
	}

	if ( m_bMeInterestPeer )
	{
		sendInterested( false );
	}

	GenAllowFastPieceList();
	//tell peer the allow fast list

	return 0;
}

int CBTPeer::DoCmdRejectRequest( void * data, int dataLen )
{


	if (!IsShaked()) 
	{
		return -1;
	}

	if ( !m_bFastExtension )
	{
		m_CloseReason=CR_PROTOCOL;
		OnClose();
		return -1;
	}

	//Reject Request: <len=0x000D><op=0x10><index><begin><offset>
	if ( dataLen != 12 )
	{
		return -1;
	}

	//m_nUploadPriority -= 2;

	//int index = ( *( ( int* ) ( data ) ) );
	int index;
	memcpy(&index, data, sizeof(int));

	index = ntohl( index );
	//unsigned int begin = ( *( ( unsigned int* ) ( ( char* ) data + 4 ) ) );
	unsigned int begin;
	memcpy(&begin, (char*)data+4, sizeof(unsigned int));

	begin = ntohl( begin );
	//unsigned int offset = ( *( ( unsigned int* ) ( ( char* ) data + 8 ) ) );
	unsigned int offset;
	memcpy(&offset, (char*)data+8, sizeof(unsigned int));

	offset = ntohl( offset );

	//check if we have send the request, if send ,then cancel all the piece's request


	if(m_MyRequest.GetIndex() == index) 
	{
		//		wchar_t msg[128];
		//		swprintf(msg,L"Peer reject my request, idx=%d,begin=%d,offset=%d",index,begin,offset);
		//		OutMsg(msg,MSG_ERROR);


		if(m_bPeerChokeMe)
		{
			//			OutMsg(L"peer choke me",MSG_INFO);
			m_MyRequest.ResetPendingRequest(begin);
			return 0;
		}
		else
		{
			//			OutMsg(L"peer not choke me",MSG_INFO);
			//����Է����Ǿܾ�������������ȡ����ͬ��������ô�ͻᵼ�¿���ѭ��
			//ȡ���񣬷������񡣡������������ܾ��ģ�����ǹر�����
			//m_pParent->GetSession() ->GetStorage() ->AbandonPieceTask( index );
			//m_MyRequest.Reset(); 
			m_CloseReason=CR_MYCLOSE;
			OnClose();
			return 0;
		}

	}

	m_LastPeerActiveTick = GetTickCount();
	return 0;

}

int CBTPeer::DoCmdAllowFast( void * data, int dataLen )
{
	if ( !m_bFastExtension )
	{
		m_CloseReason=CR_PROTOCOL;
		OnClose();
		return -1;
	}

	//Allowed Fast: <len=0x0005><op=0x11><index>

	if ( dataLen != 4 )
	{
		return -1;
	}

	//int index = ( *( int* ) data );
	int index;
	memcpy( &index, data, sizeof(int));

	index = ntohl( index );

#ifdef _CHECK	
	//OutMsg( L"got allow fast piece" );
	//PrintPeerInfo();
#endif

	//��������Ƿ�Խ��
	if(index<0||index >= int(m_pParent->GetSession() ->GetStorage()->GetPieceCount()))
	{
#ifdef _CHECK
		//OutMsg(L"allowfast index out of range!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}

	//add the piece to allowfast piece list, so can use later.
	m_AllowFastList.push_back( index );

	m_LastPeerActiveTick = GetTickCount();

	return 0;
}

//call by doWrite(), to check if we can send request packet to peer
//if can send request, send to buffer
void CBTPeer::CheckMyRequest()
{
	if ( !IsShaked() )
		return; //when got the shake from peer, our shake have sent!
	if ( !m_bMeInterestPeer ) 
		return;
	if ( m_bPeerChokeMe && !m_bFastExtension )
		return;

	bool coop=false;

	if(!m_bPeerChokeMe)
	{//���ȷ���ͨ���󣬱�chokeʱ�ٿ���allowfast����ͨ��������suggest piece

		if ( !m_MyRequest.Empty() )
		{
			DoMyRequest();
			return;
		}

		if( m_bFastExtension ) //������suggest
		{
			//check if have suggest pieces
			while ( !m_SuggestList.empty() )
			{
				int index = m_SuggestList.front();
				m_SuggestList.pop_front();


				if ( m_pParent->GetSession() ->GetStorage() ->GetPieceTask( index,coop ) )
				{ //got the task
					m_MyRequest.Init( index,
						m_pParent->GetSession() ->GetStorage() ->GetPieceLength( index ) );

					m_MyRequest.SetAllowFast(false);
					m_MyRequest.SetCoorperate(coop);

					//���ļ����ݴ���
					unsigned int voff, vlen;
					if(m_pParent->GetSession()->GetStorage()->GetAffectRangeByVirtualFileInPiece(index, voff,vlen))
					{
						m_MyRequest.SetVirtualData(voff,vlen);
					}

					//check if have orphan data
					//bool orphan = CheckOrphanData( pieceIndex );

					if ( !coop && CheckOutOrphanData() )
					{  //the orphan data make this piece completed
						//and CHeckOrphanData do all the job ,so just return
						//or get another piecetask
						return ;
					}

					DoMyRequest();
					//finish job return
					return ;
				}
				else
				{
					continue; //try next suggest piece
				}
			}//while ( !m_SuggestList.empty() )
		}

		//ȡ��ͨ���񣬵�������û�з���suggest piece ����
		//if ( m_MyRequest.Empty() )
		{
			//get other pieces from storage
			int index ;

			if ( m_pParent->GetSession() ->GetStorage() ->GetPieceTask( m_PeerBitSet, index ,coop) )
			{ //got a task

				m_MyRequest.Init( index,
					m_pParent->GetSession() ->GetStorage() ->GetPieceLength( index ) );


				m_MyRequest.SetAllowFast(false);
				m_MyRequest.SetCoorperate(coop);

				//���ļ����ݴ���
				unsigned int voff, vlen;
				if(m_pParent->GetSession()->GetStorage()->GetAffectRangeByVirtualFileInPiece(index, voff,vlen))
				{
					m_MyRequest.SetVirtualData(voff,vlen);
				}

				//check if have orphan data
				//�����������ǲ������¶�����
				if ( !coop && CheckOutOrphanData() )
				{
					return;
				}

				//send the requst to buffer
				DoMyRequest();

				//finish job return
				return ;
			}
			else
			{
				//no piece we want ,send not interest to avoid we check task again and again
				sendInterested(false);
			}

		}

	}
	else //peer choke me! only send allowfast piece
	{//��allowfast�����ܵ�����϶��ǿ�����չ�ģ������ֹ���������
		if ( m_MyRequest.Empty() )
		{
			while ( !m_AllowFastList.empty() )  //allow fast don't need to check if we choked by peer
			{

				int index = m_AllowFastList.front();
				m_AllowFastList.pop_front();

				if ( m_pParent->GetSession() ->GetStorage() ->GetPieceTask( index ,coop) )
				{ //got the task
					m_MyRequest.Init( index,
						m_pParent->GetSession() ->GetStorage() ->GetPieceLength( index ));

					m_MyRequest.SetAllowFast(true);
					m_MyRequest.SetCoorperate(coop);

					//���ļ����ݴ���
					unsigned int voff, vlen;
					if(m_pParent->GetSession()->GetStorage()->GetAffectRangeByVirtualFileInPiece(index, voff,vlen))
					{
						m_MyRequest.SetVirtualData(voff,vlen);
					}

					//check if have orphan data
					//bool orphan = CheckOrphanData( pieceIndex );

					if ( !coop && CheckOutOrphanData() )
					{  //the orphan data make this piece completed
						//and CHeckOrphanData do all the job ,so just return
						//or get another piecetask
						//OutMsg(L"orphan alread finished this piece");
						return ;
					}

					DoMyRequest();
					//finish job return
					return ;
				}
				else
				{
					continue; //try next allow fast index
				}

			}
		}
		else if(m_MyRequest.IsAllowFast())
		{
			DoMyRequest();
		}
	}
}

//call by sendbitfield
void CBTPeer::SendHaveNone()
{
	if(m_bFastExtension)
	{
		//Have None: <len=0x0001><op=0x0F>
		char buf[ 5 ];

		//*( ( unsigned int* ) buf ) = htonl( 1 );
		long tmp=htonl(1);
		memcpy(buf, &tmp, 4);

		buf[ 4 ] = char( 0x0F );

		SendData( buf, 5 );
		m_LastMyActiveTick = GetTickCount();
	}
	else
	{
		CBTPiece bitset;
		bitset.Init(m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount());

		std::string bitfield = bitset.GetStream();
		std::string buf;
		char header[ 5 ];
		//*( ( unsigned int* ) ( header ) ) = htonl( 1 + bitfield.size() );
		long tmp=htonl( 1 + bitfield.size() );
		memcpy(header, &tmp, 4);

		//*( ( char* ) ( header + 4 ) ) = 5;
		header[4]=5;

		buf.append( header, 5 );
		buf.append( bitfield.data(), bitfield.size() );

		SendData( buf.data(), buf.size() );
		m_LastMyActiveTick = GetTickCount();
	}

}

//call by sendbitfield
void CBTPeer::SendHaveAll()
{
	//Have All: <len=0x0001><op=0x0E>
	char buf[ 5 ];

	//*( ( unsigned int* ) buf ) = htonl( 1 );
	long tmp=htonl(1);
	memcpy(buf, &tmp, 4);

	buf[ 4 ] = char( 0x0E );

	SendData( buf, 5 );
	m_LastMyActiveTick = GetTickCount();
}

void CBTPeer::SendKeepAlive()
{
	char buf[ 4 ] = {0, 0, 0, 0};
	SendData( buf, 4 );
	m_LastMyActiveTick = GetTickCount();
}


//void CBTPeer::ResendRequest()
//{
//
//	m_MyRequest.ResetPendingRequest(); 
//}

//��������������滻CheckOrphanData
bool CBTPeer::CheckOutOrphanData()
{
	std::list<COrphan> orphans;
	std::list<COrphan>::iterator it;

	m_pParent->GetSession()->GetStorage()->CheckOutOrphanData(m_MyRequest.GetIndex(),orphans);

	for(it=orphans.begin(); it!=orphans.end();it++)
	{
		m_MyRequest.SetAlien(it->source,it->offset,it->data);
	}

	if(!m_MyRequest.IsFinish())
	{
		return false;
	}

	std::string piecedata=m_MyRequest.GetPieceData();


	if( Tools::SHA1String ( piecedata )
		== m_pParent->GetSession()->GetStorage() ->GetPieceHash( m_MyRequest.GetIndex() ) )
	{

		m_pParent->GetSession() ->GetStorage() ->WritePiece( m_MyRequest.GetIndex(), piecedata );
		m_MyRequest.Reset();
		return true;
	}
	else
	{
		m_MyRequest.ClearAlien(m_PeeriIP);
		return false;
	}
}

void CBTPeer::SaveOrphanToStorage()
{

	assert(!m_MyRequest.Empty());

	if(m_MyRequest.IsCoorperate()) return; //���������������ݴ����ڹ������������ɾ��

	std::list<COrphan> orphans;
	m_MyRequest.GetOrphans(orphans);

	if(orphans.empty()) return;

	assert(m_pParent->GetSession()!=NULL);
	assert(m_pParent->GetSession()->GetStorage()!=NULL);

	m_pParent->GetSession() ->GetStorage() ->CheckInOrphanData( m_MyRequest.GetIndex(), orphans );

}

void CBTPeer::NoticeNewHavePiece()
{

	SockLib::CAutoLock al(m_NewPieceNoticeListMutex);

	if(m_NewPieceNoticeList.empty()) return;

	TNewPieceNoticeList::iterator it;

	for ( it = m_NewPieceNoticeList.begin();it != m_NewPieceNoticeList.end();it++ )
	{
		CancelMyRequest(*it); 
		SendHave(*it);
	}	

	//check if we still interested peer
	if(m_bMeInterestPeer && !m_NewPieceNoticeList.empty())
	{
		if ( !m_pParent->GetSession() ->GetStorage() ->IsPieceInterest( m_PeerBitSet ) )
		{
			sendInterested(false);
			//m_bMeInterestPeer=false; //will be set in sendInterested
		}
	}	

	m_NewPieceNoticeList.clear();
}

int CBTPeer::CalculateUploadPriority()
{
	if(!m_bUploadMode)
	{
		m_nUploadPriority=m_nDownloadSum - m_nUploadSum + (m_bBrother ? 10:0);
		return m_nUploadPriority;
	}
	else
	{
		assert(m_bGotBitSet);
		assert(m_PeerBitSet.GetSize() > 0);

		float percent=float(m_PeerBitSet.GetSetedCount())/float(m_PeerBitSet.GetSize());
		int base= int(1000* exp(-((percent-50.0)*(percent-50.0)/200.0))); //��ֵ1000
		m_nUploadPriority= base+ (m_bBrother?0:1000); 
		return m_nUploadPriority;

	}
}

//�ϴ����ȼ�
int CBTPeer::GetUploadPriority()
{
	return m_nUploadPriority;
}

int CBTPeer::CalculateDownloadPriority()
{
	CBTPiece bitset;

	bitset= m_pParent->GetSession()->GetBitSet();

	unsigned int pc=bitset.GetSize();

	//�Է���������Ҫ������������������Ҫ����ѡ�������������Ҫ���˵����ǲ�����Ȥ�������ټ���
	//���򽫷Ŵ�Է��������ԣ����ǲ���Ҫ�����ݾ͵���û�У����������Ƚϸ��ӣ��ݲ�����
	//float peersum=float(m_PeerBitSet-bitset)/pc; 	

	//�µķ�����������������ѡ������أ�
	unsigned int weneed=m_pParent->GetSession()->GetStorage()->IsPieceInterest(m_PeerBitSet);
	float peersum=float(weneed)/pc;

	float mysum= float(bitset-m_PeerBitSet)/pc; //�����жԷ���Ҫ���������������������Է�ȫ�����أ���Ϊ���ǲ�֪���Է��Ƿ�ѡ��������

	//float exchange= peersum*mysum*1000; //�ܻ��ཻ���ı���
	//exchange���ֵ��0.5*0.5*1000=250 �仯��Χ[0-250]
	unsigned int now=GetTickCount();

	//all put int prefix make gcc happy

	unsigned int choketime;

	if(m_LastPieceData==0)
	{
		choketime=(now-m_nBitSetTimeTick)/1000;
	}
	else
	{
		choketime=(now - m_LastPieceData)/1000;
	}

	m_nDownloadPrority= 
		int(m_nAvDownSpeed)    //�ٶȣ��������Ҫֵ�����ⶥ��Խ��Խ��[0-]
		+int( m_nDownloadSum > 64 ? 64:m_nDownloadSum) 	//���������ȨֵӦ�÷ⶥ�������ٶ��и�����Ȩֵ[0-10]
		//+int( m_nDownloadSum > 10 ? 10:m_nDownloadSum) //��С������ֵ������
		//-int( m_bPeerChokeMe ? 0 : (now-m_nChokedTick)/1000 )  //����ʱ��������ױ���ƭ��Ӧ������һ����������ʱ��������
		-int(choketime)
		+int( m_bAccepted? 10 : 0 ) 	//����[0,10]
		+int( m_bEncryption? 4 : 0 ) 	//����[0,4]
		+int( m_bFastExtension? 3 : 0 ) 	//����Э��֧��[0,3]
		+int( peersum*200 )		//�Է����Ҹ���Ȥ��������
		+int( m_bBrother ? 10 : 0)	//�ֵ�
		+int( m_bSeed ? 100 : int(mysum * 100 * 0.2) );		//�����жԷ���Ҫ��������һ����Ҫ����[0-20] ������ã�û��Ҳ��
	//+( int(peersum * 100) > 20 ? 20 : int(peersum * 100) ) 	//�Է���������Ҫ�����ݵİٷֱ�[0-20]

	return m_nDownloadPrority;

}
//�������ȼ�������ѡ���ض�, choked ʱ�䳤��Ӧ�ڿ����У����Լ�¼�ϴα�CHOKED��ʱ��
int CBTPeer::GetDownloadPriority()
{
	return m_nDownloadPrority;
}

//check if peer piece request should send
bool CBTPeer::CheckPeerRequest(bool tikfortat)
{
	if ( m_bMeChokePeer )
	{
		return false;  //the allow fast request directly send to buffer in command process
	}

	if ( m_PeerRequestList.empty() )
	{
		return false;
	}


	//һ�δ�����෢һ�����ݰ�

	if( tikfortat && (m_nDownloadSum + m_pParent->GetSession()->GetStorage()->GetPeerCredit(m_PeeriIP) < m_nUploadSum) ) 
	{
		return false; //this function is for tik-for-tat
	}

	TPeerBlockRequest peerRequest = m_PeerRequestList.front();

	m_PeerRequestList.pop_front();

	std::string block;

	if ( m_pParent->GetSession() ->GetStorage() ->ReadData( block, peerRequest.index, peerRequest.offset, peerRequest.len ) &&
		block.size() == peerRequest.len )
	{
		SendPieceData( peerRequest.index, peerRequest.offset, block );
		m_pParent->GetSession()->SumUpload(m_PeeriIP,peerRequest.len);

		if(tikfortat) {
			m_nUploadSum+=peerRequest.len/4096;
			//report to storage

		}
		return true;
	}
	else
	{
#ifdef _CHECK
		//OutMsg(L"Error to read our data to send out",MSG_ERROR);
#endif
		return false; //some thing wrong !
	}

}

//fast extension
void CBTPeer::SendRejectRequest( int index, unsigned int offset, unsigned int length )
{
	//Reject Request
	//Reject Request: <len=0x000D><op=0x10><index><begin><offset>

	if ( !m_bFastExtension )
		return ;

	char buf[ 17 ];

	//*( ( unsigned int* ) buf ) = htonl( 13 );
	long tmp=htonl(13);
	memcpy(buf, &tmp, 4);

	//*( ( unsigned char* ) ( buf + 4 ) ) = 0x10;
	buf[4]=0x10;

	//*( ( unsigned int* ) ( buf + 5 ) ) = htonl( ( unsigned int)index );
	tmp=htonl( ( unsigned int)index );
	memcpy(buf+5, &tmp, 4);

	//*( ( unsigned int* ) ( buf + 9 ) ) = htonl( offset );
	tmp=htonl( offset );
	memcpy(buf+9, &tmp, 4);

	//*( ( unsigned int* ) ( buf + 13 ) ) = htonl( length );
	tmp=htonl( length );
	memcpy(buf+13, &tmp, 4);

	SendData( buf, 17 );
}

void CBTPeer::SendUnChoke()
{
	char buf[ 5 ];
	//*( ( int* ) buf ) = htonl( 1 );
	long tmp=htonl(1);
	memcpy(buf, &tmp, 4);

	//*( ( char* ) ( buf + 4 ) ) = 1;
	buf[4]=1;

	SendData( buf, 5 );

	m_bMeChokePeer = false;

	m_LastMyActiveTick = GetTickCount();
}

void CBTPeer::SendChoke()
{
	char buf[ 5 ];
	//*( ( int* ) buf ) = htonl( 1 );
	long tmp=htonl(1);
	memcpy(buf, &tmp, 4);

	//*( ( char* ) ( buf + 4 ) ) = 0;
	buf[4]=0;

	SendData( buf, 5 );

	m_bMeChokePeer = true;

	//fast extension choke doesn't clear the request

	//����ͻ����ܴ��������ǲ�ͬ��ע������10/24
	if ( !m_bFastExtension )
	{
		m_PeerRequestList.clear();
	}

	m_LastMyActiveTick = GetTickCount();
}

//check if this pieceindex is me allow peer to download fast?
bool CBTPeer::IsMeAllowFastPiece( int index )
{
	TAllowFastList::iterator it;

	for ( it = m_MeAllowFastList.begin();it != m_MeAllowFastList.end();it++ )
	{
		if ( *it == index )
			return true;
	}

	return false;
}


void CBTPeer::GenAllowFastPieceList()
{
	//TODO: ͬһ��IP��ַӦ���ܷ�����ͬ�ı������Է�ֹ����ͻ��˵Ķ����ƭ����

	//���ݿͻ��˵�IP��ַ���ɹ̶������У��ٵ����������ȥ�����ǵ����Ƭ������ÿ�ζ�
	//��ͬ��IP��������ͬ�ķ���

	std::string iphash;

	char ipstr[256];

	if(m_bBrother)
	{
		sprintf(ipstr, "%u:%u",m_PeeriIP,m_PeeriPort);
	}
	else
	{
		sprintf(ipstr, "%u:0",m_PeeriIP);
	}

	std::string s=ipstr;
	iphash=Tools::SHA1String(s);


	//generate allow fast piece list put into m_MeAllowFastList
	//if we don't have any finished piece, just ignore it;
	CBTPiece pc;
	pc=m_pParent->GetSession()->GetStorage()->GetBitSet();
	unsigned int pl=m_pParent->GetSession()->GetStorage()->GetTorrentFile()->GetPieceLength();

	unsigned int count=0;
	if(pl >=1024*1024) {
		count=1;
	}else if(pl>=512*1024){
		count=2;
	}else{
		count=3;
	}


	unsigned int my=pc.GetSetedCount();
	unsigned int ps=pc.GetSize();

	count=MIN(my,count);

	if(count==0) return;

	m_MeAllowFastList.clear();

	//ϣ���ǹ̶���λ�ò�����ƭ, ������16���漴��������ƥ�䵽�����Ѿ���ɵĿ�
	//�������ᣬ��˵������û�����������Ϊ�����Լ���ֻ�к��ٵ����ݡ�

	for(unsigned int i=0;i<16;i++)
	{
		//unsigned int r= *((unsigned int*)(iphash.data()+i));
		unsigned int r;
		memcpy(&r, iphash.data()+i, sizeof(unsigned int));

		//��������16���漴����,��������漴��ƥ�������е�Ƭ
		unsigned int rp=r%ps;	//ƥ�䵽���������ݷ�Χ��

		if(pc.IsSet(rp))
		{
			m_MeAllowFastList.push_back(int(rp));
			SendAllowFast(int(rp));
			if(m_MeAllowFastList.size() >= count) break;
		}

	}


}

TCloseReason CBTPeer::GetCloseReason()
{
	return m_CloseReason;
}

void CBTPeer::SendAllowFast( int index )
{
	//Allowed Fast: <len=0x0005><op=0x11><index>

	char buf[ 9 ];

	//*( ( unsigned int* ) buf ) = htonl( 5 );
	long tmp=htonl(5);
	memcpy(buf, &tmp, 4);

	buf[ 4 ] = char( 0x11 );

	//*( ( unsigned int* ) (buf+5) ) = htonl((unsigned int)index);
	tmp=htonl((unsigned int)index);
	memcpy(buf+5, &tmp, 4);

	SendData( buf, 9 );
	m_LastMyActiveTick = GetTickCount();
}

void CBTPeer::AdjustPeerPriorityForAgent()
{
	//judge the m_PeerId ,find out which agent it use and adjust it's priority

	//why use it? because some other agent use it!
	/*
	//two kind id, az serial

	For example: '-AZ2060-'... 

	known clients that uses this encoding style are: 

	'AZ' - Azureus 
	'BB' - BitBuddy 
	'CT' - CTorrent 
	'MT' - MoonlightTorrent 
	'LT' - libtorrent 
	'BX' - Bittorrent X 
	'TS' - Torrentstorm 
	'TN' - TorrentDotNET 
	'SS' - SwarmScope 
	'XT' - XanTorrent 
	'UT' - utorrent
	'BC' - bitcomet
	'FG' - flashget
	*/
	/*	
	assert(m_bGotShakeFromPeer);

	if(m_PeerId[0]=='-')
	{
	if(m_PeerId[1]=='B' && m_PeerId[2]=='C')
	{
	m_nDownloadSum-=5;
	//OutMsg(L"BitComet agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='M' && m_PeerId[2]=='M')
	{
	//OutMsg(L"Monma-bt",MSG_INFO);
	}
	else if(m_PeerId[1]=='A' && m_PeerId[2]=='Z')
	{
	m_nDownloadSum+=3;
	//OutMsg(L"Azureus agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='B' && m_PeerId[2]=='B')
	{
	//OutMsg(L"BitBuddy agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='C' && m_PeerId[2]=='T')
	{
	//OutMsg(L"CTorrent agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='M' && m_PeerId[2]=='T')
	{
	//OutMsg(L"Moonlight agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='L' && m_PeerId[2]=='T')
	{
	//OutMsg(L"libtorrent agent",MSG_INFO);
	m_nDownloadSum+=3;
	}
	else if(m_PeerId[1]=='B' && m_PeerId[2]=='X')
	{
	//OutMsg(L"Bittorrent agent",MSG_INFO);
	m_nDownloadSum+=3;
	}
	else if(m_PeerId[1]=='T' && m_PeerId[2]=='S')
	{
	//OutMsg(L"Torrentstorm agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='T' && m_PeerId[2]=='N')
	{
	//OutMsg(L"Torrent.net agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='S' && m_PeerId[2]=='S')
	{
	//OutMsg(L"SwarmScope agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='X' && m_PeerId[2]=='T')
	{
	//OutMsg(L"XanTorrent agent",MSG_INFO);
	}
	else if(m_PeerId[1]=='U' && m_PeerId[2]=='T')
	{
	//OutMsg(L"uTorrent agent",MSG_INFO);
	m_nDownloadSum+=3;
	}
	else if(m_PeerId[1]=='F' && m_PeerId[2]=='G')
	{
	//OutMsg(L"flashget agent",MSG_INFO);
	}
	else
	{
	//OutMsg(L"Unknown Azureus style agent",MSG_INFO);
	}
	}
	else
	{
	//	  another kind of id
	///	For example: 'S587----'... 
	//	
	//	  known clients that uses this encoding style are: 
	//	  
	//		'S' - Shadow's client 
	//		'U' - UPnP NAT Bit Torrent 
	//		'T' - BitTornado 
	//		'A' - ABC 
	//		Bram's client now uses this style... 'M3-4-2--'. 

	if(m_PeerId[0]=='S')
	{
	//OutMsg(L"shadow's client",MSG_INFO);
	}
	else if(m_PeerId[0]=='U')
	{
	//OutMsg(L"UPnP NAT Bit Torrent agent",MSG_INFO);
	}
	else if(m_PeerId[0]=='T')
	{
	//OutMsg(L"BitTornado agent",MSG_INFO);
	}
	else if(m_PeerId[0]=='A')
	{
	//OutMsg(L"ABC agent",MSG_INFO);
	}
	else if(m_PeerId[0]=='M')
	{
	//OutMsg(L"Bram's client",MSG_INFO);
	}
	else if(m_PeerId[0] >'A' && m_PeerId[0] <='Z')
	{
	//OutMsg(L"Unknown shadow style agent",MSG_INFO);
	//m_nUploadPriority-=10;
	}
	else
	{//not shadow style
	//OutMsg(L"Unknown style agent",MSG_INFO);
	//m_nUploadPriority-=20;
	}

	}
	*/

}


//Peers supporting the DHT set the last bit of the 8-byte reserved flags 
//exchanged in the BitTorrent protocol handshake. Peer receiving a 
//handshake indicating the remote peer supports the DHT should send a PORT 
//message. It begins with byte 0x09 and has a two byte payload containing 
//the UDP port of the DHT node in network byte order. Peers that receive 
//this message should attempt to ping the node on the received port and 
//IP address of the remote peer. If a response to the ping is recieved, 
//the node should attempt to insert the new contact information into their
// routing table according to the usual rules.

int CBTPeer::DoCmdDHTPort(void *data, size_t dataLen)
{


	if (!IsShaked()) 
	{
		return -1;
	}

	if ( dataLen != 2 )
	{
		return -1;
	}

	//unsigned short iport = *( ( unsigned short* ) data );
	unsigned short iport;
	memcpy(&iport, data, sizeof(unsigned short));

	//����������
	m_pParent->GetSession()->GetStorage()->NewDHTNode(m_PeeriIP,iport);

	return 0;
}

//������չ����
int CBTPeer::DoCmdPort(void *data, size_t dataLen)
{

	if (!IsShaked()) 
	{
		return -1;
	}

	if ( dataLen != 2 )
	{
		return -1;
	}


	//unsigned short iport = *( ( unsigned short* ) data );
	unsigned short iport;
	memcpy(&iport, data, sizeof(unsigned short));

	//�Է���������Ķ˿�
	//����������
	m_pParent->GetSession()->GetStorage()->NewBenliudPeer(m_PeeriIP,iport);
	return 0;
}
//set if this connection should encrypt and which side we are
//if we are connect initial then A
//if the socket is accepted then B
//called before  connecting
void CBTPeer::SetEncrypt(bool encrypt, bool isA)
{
	m_bEncryption=encrypt; //
	m_bIsA=isA;		//are we at A side?
}

//only as A need this to send initial packet
void CBTPeer::SendPublicKey()
{

	MSE::GeneratePublicPrivateKey(m_MSE_PrivateKey,m_MSE_PublicKey);

	unsigned char buf[96+512]; //max 512 byte pad
	unsigned int pada=((unsigned int)(rand()))%512;

	for(unsigned int i=0;i<pada;i++)
	{
		buf[96+i]=(unsigned char)(rand()%0xFF);
	}


	m_MSE_PublicKey.toBuffer(buf,96);

	SendData( buf, 96+pada);  //send DH public key, wait for B's public key
	m_bGotDHSecret=false;
	m_MSE_State=MSE_SEND_PUB;

}

//�����棬�����������������Ҫ�Ϸ�ʽ����
//���ؼ٣�û��������Ҫ��������
bool CBTPeer::DoDHSecretShake()
{
	assert(m_bIsA);

	switch(m_MSE_State)
	{
	case MSE_INIT: //should not happen
		{
			//�����ǶԷ������ӽ���ʱ�������������֣����Ƕ�û�з����ʼ����
			//��ô��һ������ⶼ����ͨ���֣�Ӧ�ò���������Ҫ�ļ������ַ���
			//�Է�Ӧ��û�м������ӵ�����
			m_CloseReason=CR_NOENCRYPT;
			OnClose();
			
			if(m_bReportEncryptable)
			{
				m_pParent->GetSession()->PeerSupportEncryption(m_PeeriIP,false);
			}

			return false;
		}
	case MSE_SEND_PUB:
		MSE_AfterSendPub();
		break;
	case MSE_GOT_PUB:
		if(MSE_AfterGotPub())
		{
			return true;
		}
		break;
	case MSE_FOUND_VC:
		if(MSE_AfterFoundVC())
		{
			return true; 
		}
		break;
	case MSE_WAIT_PAD_D:
		if(MSE_AfterWaitPadD())
		{
			return true;
		}
		break;
	case MSE_FOUND_REQ1: //no use ,make gcc happy
	case MSE_WAIT_PAD_C:
	case MSE_WAIT_IA:
	case MSE_FINISH:
		break;
	}

	return false;

}


bool CBTPeer::MSE_AfterSendPub()
{
	if(m_bIsA)
	{

		//���Ƿ���ļ������ӣ��Է�������ȴ���ֱ�ӷ�����ͨ���֣�������ܻ��յ�68�ֽڵ�����
		//����������Ļ������ܻ�ɵ�����������Ҫ�жϣ���������������ʵ���϶Է�����֧�ּ�������

		if(m_recvBuffer.size() >=20)
		{
			std::string check=m_recvBuffer.substr(1,19);

			if(m_recvBuffer[0]==19)
			{
#if defined( WIN32)||defined(WINCE)
				if ( _stricmp( check.c_str(), "BitTorrent protocol" ) != 0 )
#else
				if ( strcasecmp( check.c_str(), "BitTorrent protocol" ) != 0 )
#endif
				{
					//yes , it's no encryption style shake return;
					//					OutMsg(L"peer have no encryption support!!",MSG_ERROR);
					m_CloseReason=CR_NOENCRYPT;
					OnClose();

					if(m_bReportEncryptable)
					{
						m_pParent->GetSession()->PeerSupportEncryption(m_PeeriIP,false);
					}

					return false;
				}
			}
		}

		//Ҫ�ȴ�����
		//2 B->A: Diffie Hellman Yb, PadB	 [����96-608] 
		if(m_recvBuffer.size()<96) 
		{
			//we are waiting pub key
			return false;
		}


		MSE::BigInt peerpub=MSE::BigInt::fromBuffer((const unsigned char*)(m_recvBuffer.data()),96); //build the public key
		m_MSE_DHSecret=MSE::DHSecret(m_MSE_PrivateKey,peerpub);
		m_bGotDHSecret=true;
		m_MSE_State=MSE_GOT_PUB;


		MSE::BTDHTKey infohash(m_pParent->GetSession()->GetStorage()->GetTorrentFile()->GetInfoHash().data());

		MSE::BTDHTKey keya=MSE::EncryptionKey(true,m_MSE_DHSecret,infohash);  //A�ļ�����Կ
		MSE::BTDHTKey keyb=MSE::EncryptionKey(false,m_MSE_DHSecret,infohash); //B�ļ�����Կ

		m_MSE_pEncryptor=new MSE::RC4Encryptor(keyb,keya);//A�ļ��ܽ�����

		m_MSE_pPeerEncryptor=new MSE::RC4Encryptor(keya,keyb); //B�ļ��ܽ�����
		/*
		//The first 1024 bytes of the RC4 output are discarded. ???!!!!
		unsigned char discarded[1024];
		m_Encryptor->encryptReplace(discarded,1024); 
		m_Encryptor->decrypt(discarded,1024);

		m_MSE_pPeerEncryptor->encryptReplace(discarded,1024);
		m_MSE_pPeerEncryptor->decrypt(discarded,1024);
		*/

		//ģ��Է�����ENCRYPT(VC),��������ͬ����
		//���������������A�������ǲ�����1K��B��Ӧ���ܹ�ʶ������û�ж���1K
		//Ȼ��������ǵļ��ܷ���������ȷ��VC���ܴ�������Ӧ����1K���ټ���
		//�������ǿ��Լ���Է��ǲ��ᶪ��1K�ģ������ȷʵ�����ˣ������Ҳ���VC��
		//˵�����ǺͶԷ�û�к��ģ�ֻ�е����������ǣ���Ϊ���ǿ���ʶ�����Ƿ���1K����
		memset(m_MSE_ENCRYPTVC,0,8);
		m_MSE_pPeerEncryptor->encryptReplace(m_MSE_ENCRYPTVC,8);

		delete m_MSE_pPeerEncryptor;
		m_MSE_pPeerEncryptor=NULL;

		//�õ�����Կ������ȷ��
		//3 A->B: 
		//HASH('req1', S), 
		//HASH('req2', SKEY) xor HASH('req3', S), 
		//ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)

		unsigned char s1hash[20];
		unsigned char s2hash[20];
		unsigned char buf[1024];

		//HASH('req1', S), 
		memcpy(buf,"req1",4);
		m_MSE_DHSecret.toBuffer(buf+4,96);

		HashLib::CSHA1 tmp;
		tmp.Hash((const char*)buf, 100);
		memcpy(s1hash, tmp.GetHash(), tmp.GetHashLen());
		//SHA1Block(buf,100,s1hash);
		SendData(s1hash,20);

		//HASH('req2', SKEY) xor HASH('req3', S), 
		memcpy(buf,"req2",4);
		memcpy(buf+4,m_pParent->GetSession()->GetStorage()->GetTorrentFile()->GetInfoHash().data(),20);
		tmp.Hash((const char*)buf, 24);
		memcpy(s1hash, tmp.GetHash(), tmp.GetHashLen());
		//SHA1Block(buf,24,s1hash);

		memcpy(buf,"req3",4);
		m_MSE_DHSecret.toBuffer(buf+4,96);
		tmp.Hash((const char*)buf,100);
		memcpy(s2hash, tmp.GetHash(), tmp.GetHashLen());
		//SHA1Block(buf,100,s2hash);

		for(int i=0;i<20;i++)
		{
			s1hash[i]=s1hash[i]^s2hash[i];
		}
		SendData(s1hash,20);

		//ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
		//IA = initial payload data from A
		//may be 0-sized if you want to wait for the encryption negotation.

		memset(buf,0,8);	//VC
		//		memset(buf+8,0,4);  //crypto_provide

		unsigned int crypto_provide=3;  //plain text and rc4 support, let B choose cryto

		crypto_provide=htonl(crypto_provide);
		memcpy(buf+8,&crypto_provide,4);


		unsigned short padc= (unsigned short)(rand()%512); 

		for(unsigned short k=0;k<padc;k++)
		{
			buf[8+4+2+k]=rand()%0xFF;
		}

		unsigned int iapos=8+4+2+padc;

		padc=htons(padc);
		memcpy(buf+8+4,&padc,2);

		m_MSE_pEncryptor->encryptReplace(buf, iapos);

		SendData(buf,iapos);

		//���Ҫ�����ְ�����ia=68, ���ְ�
		//����ia=0

		//�������Ƿ�һ�����ְ�


		//skip to len(IA), ignore PadC
		unsigned short ia=htons(68); //set len(IA) to 68

		memcpy(buf,&ia,2); 
		m_MSE_pEncryptor->encryptReplace(buf, 2);
		SendData(buf,2);  //����len(IA)

		char shakebuf[68];
		MakeShake(shakebuf);

		m_MSE_pEncryptor->encryptReplace((unsigned char*)shakebuf, 68);

		//send out
		SendData(shakebuf,68);

		m_bSendShakeToPeer=true;
		//switch status

		m_MSE_State=MSE_GOT_PUB;//???
		//m_MSE_State=MSE_FINISH;

		return false;
	}
	else
	{//B�Ĵ�������
		//���ﴦ�����������ҵ�ͬ���㣬���ֶ�Ӧ������SKEY=infohash
		//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S)
		//HASH('req1',S)��������ͬ������HASH('req2', SKEY)���������ж��Ƕ�Ӧ�ĸ�����
		//S=m_MSE_DHSecret,SKEY=infohash		
		//HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
		if(m_recvBuffer.size() < 96 + 20)
		{
			return false;
		}

		int end=m_recvBuffer.size()-96;

		for(int i=0; i<end-20; i++)
		{
			if(memcmp(m_MSE_REQ1HASH,m_recvBuffer.data()+96+i,20) == 0)
			{
				m_MSE_REQ1POS=i+96; //����λ��
				m_MSE_State=MSE_FOUND_REQ1;
				break;
			}
		}

		if(m_MSE_State!=MSE_FOUND_REQ1)
		{
			if(m_recvBuffer.size() >= 96 +512 + 20)
			{
				m_CloseReason=CR_PROTOCOL;
				OnClose();
				return false;

			}

			return false;

		}

		return MSE_AfterFoundReq1();
	}
}

bool CBTPeer::MSE_AfterGotPub()
{
	assert(m_bIsA);

	if(m_recvBuffer.size() < 96 + 8)
	{
		//too short ,no vc
		return false;
	}

	int end=m_recvBuffer.size()-96;

	for(int i=0; i<end-8; i++)
	{
		if(memcmp(m_MSE_ENCRYPTVC,m_recvBuffer.data()+96+i,8) == 0)
		{
			m_MSE_VCPOS=i+96; //����λ��
			m_MSE_State=MSE_FOUND_VC;
			break;
		}
	}

	if(m_MSE_State!=MSE_FOUND_VC)
	{
		if(m_recvBuffer.size() >= 96 +512 + 8)
		{
			m_CloseReason=CR_PROTOCOL;
			OnClose();
			return false;

		}

		return false;

	}

	return MSE_AfterFoundVC();


}

//ֻ��A��Ҫ���ENCRYPT(VC)��ͬ��
bool CBTPeer::MSE_AfterFoundVC()
{
	assert(m_bIsA);

	//check if ENCRYPT(VC, crypto_select, len(padD)) got
	//��鱾���Ƿ���Եõ�ENCRYPT(VC, crypto_select, len(padD))

	if(m_recvBuffer.size()-m_MSE_VCPOS < 8+4+2)
	{
		return false; //ENCRYPT(VC, crypto_select, len(padD)) ������
	}

	m_recvBuffer.erase(0, m_MSE_VCPOS); //now can safely remove the public key and the padB

	//the left is :
	//ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
	unsigned char swap[8+4+2];
	memcpy(swap,m_recvBuffer.data(),8+4+2); //copy VC+crypto_select,len(padD) 

	m_recvBuffer.erase(0, 8+4+2);	//����padd
	m_MSE_State=MSE_WAIT_PAD_D;

	m_MSE_pEncryptor->decrypt(swap,8+4+2);

	//unsigned int crypto_select=*((unsigned int*)(swap+8));
	unsigned int crypto_select;
	memcpy(&crypto_select, swap+8, sizeof(unsigned int));

	crypto_select=ntohl(crypto_select);

	//����ԭ����ѡ�ǲ����ܣ���ѡ���ܣ����������ԭ����Լ��ԽӲ���
	if(crypto_select & 1)
	{
		m_bFullEncryption=false; //�Ժ󲻼���
	}
	else if(crypto_select & 2)
	{
		m_bFullEncryption=true; //�Ժ����
	}
	else
	{

		m_CloseReason=CR_PROTOCOL;

		OnClose();

		return false;
	}

	//unsigned short padd=*((unsigned short*)(swap+8+4));
	unsigned short padd;
	memcpy(&padd, swap+8+4, sizeof(unsigned short));

	m_MSE_PADDLEN = ntohs(padd); //assert padlen < 512 

	if(m_MSE_PADDLEN >512)
	{

		m_CloseReason=CR_PROTOCOL;
		OnClose();

		return false;
	}



	return MSE_AfterWaitPadD();

}

//ֻ�����ӷ����߲ŵȴ������������
//4 B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
bool CBTPeer::MSE_AfterWaitPadD()
{
	assert(m_bIsA);

	if(m_MSE_PADDLEN==0)
	{

		if(m_bFullEncryption && !m_recvBuffer.empty())
		{

			m_MSE_State=MSE_FINISH; //�������ֹ��̽���

			int leftlen=m_recvBuffer.size();

			unsigned char *left=new unsigned char[leftlen+2];
			memcpy(left,m_recvBuffer.data(),leftlen);
			m_MSE_pEncryptor->decrypt(left,leftlen);
			m_recvBuffer.resize(0);
			m_recvBuffer.append((const char*)left,leftlen);
			delete[] left;

			return true;
		}
		else if(m_bFullEncryption && m_recvBuffer.empty())
		{
			m_MSE_State=MSE_FINISH; //�������ֹ��̽���
			return false;
		}
		else if(!m_bFullEncryption)
		{
			m_MSE_State=MSE_FINISH; //�������ֹ��̽���
			return !m_recvBuffer.empty();
		}

	}

	if(m_recvBuffer.size()<m_MSE_PADDLEN)
	{
		return false;
	}

	//����PADD��Ȼ����padD��û���������ͬ������
	unsigned char abpadd[512];
	m_MSE_pEncryptor->decrypt(abpadd,m_MSE_PADDLEN);

	//ɾ��PADD����
	m_recvBuffer.erase(0,m_MSE_PADDLEN);


	//��û��ʣ�����ݣ�
	if(m_recvBuffer.empty())
	{
		m_MSE_State=MSE_FINISH; //�������ֹ��̽���
		return false;
	}
	else
	{
		//		OutMsg(L"encrypt shake end,have payload stream");

		//ʣ������Ӧ����ENCRYPT2(Payload Stream)

		if(m_bFullEncryption)
		{	
			//			OutMsg(L"padd is full encryption, decrypt it put back to buffer.");
			//�����ƶ���read��������Ҫ����ʣ��Ĳ���
			m_MSE_State=MSE_FINISH; //�������ֹ��̽���

			int leftlen=m_recvBuffer.size();

			unsigned char *left=new unsigned char[leftlen+2];
			memcpy(left,m_recvBuffer.data(),leftlen);
			m_MSE_pEncryptor->decrypt(left,leftlen);
			m_recvBuffer.resize(0);
			m_recvBuffer.append((const char*)left,leftlen);
			delete[] left;

			//			wchar_t msg[256];
			//			swprintf(msg,L"payload len=%d",leftlen);
			//			OutMsg(msg);

			return true;
		}
		else
		{//����ȫ���ܣ�ʣ��Ķ���û���ܵ�

			m_MSE_State=MSE_FINISH;

			return true;

		}

	}

}

bool CBTPeer::IsEncrypt()
{
	return m_bEncryption;
}

//directly check the m_recvbuffer for shakehand
bool CBTPeer::CheckAcceptedShakeHand()
{
	assert(m_bAccepted);
	assert(!m_bIsA);

	if(m_recvBuffer.size() < 68)
	{
		//��Щ�ǻ���ֻ����48�ֽڣ�������peer_id���ڣ�����Ҳ���Դ���
		//��������ʱ��68�ֽ�������

		return false;
	}

	bool bMSE=false;
	std::string oldstyle;
	oldstyle=m_recvBuffer.substr(0,68);


#if defined( WIN32)||defined(WINCE)
	if ( oldstyle[ 0 ] != 19 || _stricmp( oldstyle.substr( 1, 19 ).c_str(), "BitTorrent protocol" ) != 0 )
#else
	if ( oldstyle[ 0 ] != 19 || strcasecmp( oldstyle.substr( 1, 19 ).c_str(), "BitTorrent protocol" ) != 0 )
#endif
	{
		//treat as MSE
		bMSE=true;
	}

	if(!bMSE)
	{

		if ( oldstyle[ 27 ] & 0x04 )
		{
			m_bFastExtension = true;  //fast extension enabled!
		}

		if( oldstyle[ 26 ] & 0x01 )
		{
			m_bPortExchange=true;
		}

		if( oldstyle[25] & 0x10)
		{
			m_bUtPex=true;  //peer support ut_pex
		}

		std::string peerhash = oldstyle.substr( 28, 20 );

		//���ڼ���ʱ����������infohash, Ҫ�������͸�����֪ͨ��Ҳ����Ҫ����ת���ˡ�
		//ת�ƺ������m_pParent ,���û��ת�������GetSession()����NULL,�����

		//���infohash, ����ת�ƣ�ת�ƺ�parent��Ӧ�ı���
		if(!m_pParent->GotHash(peerhash,this))  //����ᷢ��HASHУ��
		{
			//HASHУ��ʧ�ܻ��޷���������
			m_CloseReason=CR_MYCLOSE;
			OnClose();
			return false;
		}


		m_PeerId = oldstyle.substr( 48, 20 );

		m_bGotShakeFromPeer = true;


		//���û������m_pParentΪvolatile���ͣ�������ܻ����������ϵ�ָ��
		//��Ϊ���ָ�����һֱ�ڼĴ�����
		//check if this peer is myself
		if(m_pParent->GetSession()->IsSelfPeerId(m_PeerId))
		{
			m_CloseReason=CR_SELF;
			OnClose();

			return false;

		}


		m_nShakeTimeTick=GetTickCount(); //record the shake time

		CheckAgent(); //check on m_PeerId

		//send out our shake hand
		SendHandshake();


		//�����ǽ����ߣ����ȷ���bitset

		m_LastMyActiveTick = GetTickCount();

		//remove the handshake msg
		m_recvBuffer.erase(0,68);

		return true;

	}
	else
	{
		//treat as MSE,
		m_bEncryption=true;
		//m_bIsA=false;
		return MSE_AfterConfirmEncrypt();

	}
}

bool CBTPeer::MSE_AfterFoundReq1()
{//only B

	assert(!m_bIsA);

	int leftbyte= m_recvBuffer.size() - m_MSE_REQ1POS;

	if(leftbyte < 20+20+8+4+2) return false;


	//���ڼ����ϱ����������ж϶�Ӧ���ĸ�����
	//ǰ20�ֽ�HASH('req1', S)����20�ֽ�HASH('req2', SKEY) xor HASH('req3', S)
	//��20�ֽ������ж����������ĸ����񣬴�ʱm_MSE_pEncryptor==NULL

	//m_recvBuffer.erase(0, m_MSE_REQ1POS);
	//std::string hash1=m_recvBuffer.substr(0,20);
	//m_recvBuffer.erase(0,20);
	//std::string hash2=m_recvBuffer.substr(0,20);
	//m_recvBuffer.erase(0,20);

	m_recvBuffer.erase(0, m_MSE_REQ1POS+20);  //ǰ20�ֽ���HASH('req1',S) ����Ҫ������һ���������Ѿ�ȷ����
	std::string hashxor=m_recvBuffer.substr(0,20); //��20�ֽ���HASH('req2', SKEY) xor HASH('req3', S)�������ж��������
	m_recvBuffer.erase(0,20); //������ɾ��������HASH��ʣ��ľ��Ǽ�������ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)

	//���HASH('req2', SKEY) xor HASH('req3', S)���ж��������
	//���ǲ�֪������Щ����ע���ˣ�ֻ���ύ��listener���ж����HASH��������ĸ�����
	if(!m_pParent->GotEncryptHash( hashxor, m_MSE_DHSecret ,this))
	{//����ע��������񣬹ر�����
		assert(m_LinkStatus==LS_CONNOK);

		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return false;
	}



	//���ڶ�ڼ����͵��ڼ������ƶ�������������m_MSE_pEncryptor��

	//���GotEncryptHash�����棬�����˹���ת�ƣ��������ﶼ��ת���˵ġ�

	//�������Ѿ������˹����л������Լ���m_MSE_pEncryptor��
	//��Ϊ�л�����Ϳ��Եõ�infohash
	MSE::BTDHTKey infohash(m_pParent->GetSession()->GetStorage()->GetTorrentFile()->GetInfoHash().data());

	MSE::BTDHTKey keya=MSE::EncryptionKey(true,m_MSE_DHSecret,infohash);  //A�ļ�����Կ
	MSE::BTDHTKey keyb=MSE::EncryptionKey(false,m_MSE_DHSecret,infohash); //B�ļ�����Կ

	m_MSE_pEncryptor=new MSE::RC4Encryptor(keya,keyb); //���ǣ�B���ļ��ܽ�����,���������B	

	unsigned char ency[8+4+2];
	memcpy(ency,m_recvBuffer.data(),8+4+2);

	//���Բ�����1K�ֽڵĽ��ܣ������ܲ��ܵõ�ȫ0��8�ֽ�VC
	unsigned char vc[8];
	memcpy(vc,ency,8);
	m_MSE_pEncryptor->decrypt(vc,8);

	for (int j=0;j<8;j++)
	{
		if(vc[j])
		{
			//���ԣ�����1K�ֽ�
			unsigned char discard[1024-8];
			m_MSE_pEncryptor->decrypt(discard,1024-8);
			m_MSE_Drop1K=true;
			break;
		}
	}

	if(m_MSE_Drop1K)
	{

		//�ٴν���
		m_MSE_pEncryptor->decrypt(ency,8+4+2);
		//check the ENCRY(VC) 8 byte 0
		bool anywrong=false;
		for (int j=0;j<8;j++)
		{
			if(ency[j])
			{
				anywrong=true;
				break;
			}
		}

		if(anywrong)
		{//���ǽ���ʧ�ܣ�����

			m_CloseReason=CR_PROTOCOL;
			OnClose();
			return false;
		}
		else
		{
			//discard our 1024 bytes
			//�ɹ�������1K�����ֽ�
			unsigned char dis[1024];
			m_MSE_pEncryptor->encryptReplace(dis,1024);
		}

	}
	else
	{
		//���趪��1K���ܿ��Գɹ�����������ʣ��6�ֽ�
		m_MSE_pEncryptor->decrypt(ency+8,4+2);
	}


	//unsigned int crypto_provide=*((unsigned int*)(ency+8));
	unsigned int crypto_provide;
	memcpy(&crypto_provide, ency+8, sizeof(unsigned int));

	crypto_provide=ntohl(crypto_provide);


	if(crypto_provide & 0x2 )  //set the rc4
	{
		m_bFullEncryption=true;
	}
	else
	{
		m_bFullEncryption=false; //IA�����Ͳ��ý�����
	}

	//unsigned short padc=*((unsigned short*)(ency+12));
	unsigned short padc;
	memcpy(&padc, ency+12, sizeof(unsigned short));

	m_MSE_PADCLEN=ntohs(padc);

	if(m_MSE_PADCLEN > 512) 
	{
		m_CloseReason=CR_PROTOCOL;
		OnClose();
		return false;
	}

	m_MSE_State=MSE_WAIT_PAD_C;

	m_recvBuffer.erase(0,8+4+2);

	return MSE_AfterWaitPadC();
}

//ֻ��B����Ҫ��������������ȴ�PADC
//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
bool CBTPeer::MSE_AfterWaitPadC()
{
	assert(!m_bIsA);

	if(m_recvBuffer.size() < m_MSE_PADCLEN + 2) return false; //������ͬlen(IA)һ��ȴ�

	unsigned char padc[512+2];
	memcpy(padc,m_recvBuffer.data(),m_MSE_PADCLEN+2);
	m_MSE_pEncryptor->decrypt(padc,m_MSE_PADCLEN+2);

	m_recvBuffer.erase(0,m_MSE_PADCLEN+2);	//padc �� len(IA)һ��ɾ��

	unsigned short ialen;
	memcpy(&ialen,padc+m_MSE_PADCLEN,2);

	m_MSE_IALEN=ntohs(ialen);	//���IA���ȣ������68�����߼���bitfield����

	//���ڳ�����ʱ���ܴ�������Я����bitset
	//�����ܷ�����������Ϊ��������ת�ƺ�Ĵ��������ɼ����߳�������bitset�仯�ܸ��ӣ�����������ץס����������
	//���IA!=0 and IA!=68ֱ�ӹر����ӣ������ü����߳�ִ��bitset�仯����
	//�����ﶼ�Ƿ�����ת�Ƶģ���ִ���߳����Ǽ����߳�

#ifdef _CHECK
	if(m_MSE_IALEN!=0 && m_MSE_IALEN!=68)
	{
		//m_CloseReason=CR_MYCLOSE;
		//OnClose();
		//return;
		//OutMsg(L"IA!=0 && IA!=68", MSG_WARNNING);  //
		if(m_MSE_IALEN > 68) 
		{
			//syslog("IA len > 68!! careful\n");
			//OutMsg(L"IA>68",MSG_WARNNING);
		}
	}
#endif
	//Ϊ���ȶ���ֱ�ӹرմ�bitset��IA���ӣ������ֲ������ԭ���µ��ȶ�����ʱ������ȡ���������
	if(m_MSE_IALEN!=0 && m_MSE_IALEN!=68)
	{
		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return false; //��Ҫ�ü����߳�ȥִ��bitset�����ˡ�
	}

	//����ȷ�ϰ�������PAYLOAD����Ϊ��ΪB����ҪA�ȷ�IA��
	//�´λ����IA�����ǲŷ��͡��������ǲ�֪���Է��Ƿ���IA����
	//�ȷ�B->A: ENCRYPT(VC, crypto_select, len(padD), padD) �ⲿ��
	//B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)

	unsigned char rep[8+4+2+512];
	memset(rep,0,8);	//vc

	unsigned int crypto_select= 2; //ѡ��ȫ���ܷ�ʽ

	if(!m_bFullEncryption) crypto_select=1; //�Ժ󲻼��ܷ�ʽ

	crypto_select=htonl(crypto_select);
	memcpy(rep+8,&crypto_select,4);

	unsigned short padd=(rand()%512);

	//����padd���
	for(unsigned short k=0;k<padd;k++)
	{
		rep[8+4+2+k]=rand()%0xFF;
	}

	unsigned int totallen=8+4+2+padd;

	padd=htons(padd);

	memcpy(rep+8+4,&padd,2);

	//��������
	m_MSE_pEncryptor->encryptReplace(rep,totallen);

	//���ͼ������ݣ������ﲻ���м��ܴ���
	SendData(rep,totallen); 


	//�´εȴ�IA
	m_MSE_State=MSE_WAIT_IA;
	return MSE_AfterWaitIA();


}

//ֻ��B����Ҫ��������������ȴ�IA
bool CBTPeer::MSE_AfterWaitIA()
{
	assert(!m_bIsA);

	if( m_MSE_IALEN ==0 )
	{
		m_MSE_State=MSE_FINISH;
		return false;
	}

	if(m_recvBuffer.size() < m_MSE_IALEN) 
	{
		return false;
	}

	if(!m_bFullEncryption)
	{
		//decrypt it and return true;
		std::string copy=m_recvBuffer;
		copy.erase(0,m_MSE_IALEN);

		unsigned char *ia=new unsigned char[m_MSE_IALEN+2];
		memcpy(ia,m_recvBuffer.data(),m_MSE_IALEN);
		m_MSE_pEncryptor->decrypt(ia,m_MSE_IALEN);

		m_recvBuffer.resize(0);
		m_recvBuffer.append((char*)ia,m_MSE_IALEN);
		m_recvBuffer+=copy;

		m_MSE_State=MSE_FINISH;
		delete[] ia;

		return true;
	}
	else
	{
		//ȫ����

		//�������ݷ���
		size_t left=m_recvBuffer.size();

		unsigned char *ia=new unsigned char[left+2];
		memcpy(ia,m_recvBuffer.data(),left);

		m_MSE_pEncryptor->decrypt(ia,left);

		m_recvBuffer.resize(0);
		m_recvBuffer.append((char*)ia,left);

		m_MSE_State=MSE_FINISH;
		delete[] ia;

		//ia����������ְ���bitfield�������������ֱ�Ӻ��ڴ����������Ժ�������Ҳ���Զ����ܵ���
		return true;
	}

}

bool CBTPeer::MSE_AfterConfirmEncrypt()
{//this function is for we are B, the socket is accepted

	assert(!m_bIsA);

	if(m_recvBuffer.size()<96) {
		m_MSE_State=MSE_INIT;
		return false; //wait for data
	}

	//we can got the public key now
	MSE::BigInt peerpub=
		MSE::BigInt::fromBuffer((const unsigned char*)(m_recvBuffer.data()),96); //build the public key

	//cal the m_MSE_DHSecret

	//gen our private and public key send back
	MSE::GeneratePublicPrivateKey(m_MSE_PrivateKey,m_MSE_PublicKey);

	m_MSE_DHSecret=MSE::DHSecret(m_MSE_PrivateKey,peerpub);

	m_bGotDHSecret=true;
	m_MSE_State=MSE_GOT_PUB;

	//req1
	unsigned char req1[100];
	memcpy(req1,"req1",4);
	m_MSE_DHSecret.toBuffer(req1+4,96);

	HashLib::CSHA1 tmp;
	tmp.Hash((const char*)req1,100);
	memcpy(m_MSE_REQ1HASH, tmp.GetHash(), tmp.GetHashLen());
	//SHA1Block(req1,100,m_MSE_REQ1HASH);

	//���ڵ��ڼ���������������ǵĽ�������ʱ�����ԣ���Ϊ���ǲ�֪��������Ӷ�Ӧ�����ĸ�����
	//һ��Ҫ�ȶԷ�ָ����Ҫ���ӵ������������л�������������m_MSE_pEncryptor
	//���������������ֻ����Կ���Է�
	//2 B->A: Diffie Hellman Yb, PadB	 [����96-608] //B���ܷ��ͳ���608�ֽ�, 30����Ҳ������96�ֽ�,����Ӧ�ùر�����

	//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S)
	//HASH('req1',S)��������ͬ������HASH('req2', SKEY)���������ж��Ƕ�Ӧ�ĸ�����
	//S=m_MSE_DHSecret,SKEY=infohash
	//��Щ�������´ε�A->B��������ȷ�������ȷ�����˵�
	//�´�A->B�������ٹ���m_MSE_pEncryptor


	unsigned char buf[96+512]; //max 512 byte pad
	unsigned int padb=((unsigned int)(rand()))%512;

	for(unsigned int i=0;i<padb;i++)
	{
		buf[96+i]=(unsigned char)(rand()%0xFF);
	}

	m_MSE_PublicKey.toBuffer(buf,96);

	SendData( buf, 96+padb);  //send DH public key, wait for B's public key

	m_MSE_State=MSE_SEND_PUB;

	//��������и��жϣ�����Ҫ��������������
	return false; //false???
}

bool CBTPeer::IsGotBitSet()
{
	return m_bGotBitSet;
}

bool CBTPeer::IsMeInterestPeer()
{
	return m_bMeInterestPeer;
}

bool CBTPeer::IsChokedByPeer()
{
	return m_bPeerChokeMe;
}

unsigned int CBTPeer::GetShakeTime()
{
	return m_nShakeTimeTick; //if return 0 ,not shake
}

bool CBTPeer::IsShaked()
{
	return m_bGotShakeFromPeer && m_bSendShakeToPeer;
}

int CBTPeer::DoPexCommand(void *data, size_t datalen)
{
//just believe the peer port in shake ,ignore all the others
 //the shake command have id=0, id is the first byte.

	if(!IsShaked()) return -1;
	if(!m_bAccepted) return -1;

	if(!m_bUtPex) 
	{
		return -1;
	}

	char *pbuf= (char*)data;
	if(*pbuf!=0) {
		return 0; //not a shake message, ignore.
	}

	pbuf++; //move to the dictionary

	if(datalen <=1) return -2; //avoid some error.

	BencodeLib::CTorrentFile tf;
	if(0!=tf.ReadBuf(pbuf, datalen-1))
	{
		return -3;
	}

	//find the port
	BencodeLib::CBenNode& ben=tf.GetRootNode();
	BencodeLib::CBenNode* pport=ben.FindKeyValue("p");
	if(pport==NULL) 
	{
		return -2;
	}

	llong port=pport->GetIntValue();
	if(port <= 1024 || port >=65536)
	{
		return -4;
	}

	unsigned short lport=(unsigned short)port;
	lport=htons(lport);


#ifdef _CHECK
	//OutMsg(L"add a new peer from ut_pex",MSG_INFO);
#endif
	m_pParent->GetSession()->GetStorage()->AddNewPeer(m_PeeriIP,lport);
	
	return 0;
}

void CBTPeer::SetUploadMode(bool upload)
{
	m_bUploadMode=upload;
}

void CBTPeer::JobEndClose()
{
	//remove all the timer
	Close();	

	m_ActiveCheckTimer = 0;
	m_NewPieceNoticeCheckTimer = 0;

}

bool CBTPeer::IsSeed()
{
	return m_bSeed;
}


//use m_NewPieceNoticeCheckTimer to do this work to caculate last 2min speed
void CBTPeer::CalculateSpeed()
{
#define AVTIME	(20)	//ƽ���ٶȼ���ʱ��

	//�·�����¼һ���ӵ�ƽ���ٶ�
	unsigned int now=GetTickCount();
	TReceivedDataRecord rdr;
	rdr.amount=m_nDownloadSum;
	rdr.timetick=now;
	m_SpeedList.push_back(rdr);

	while(!m_SpeedList.empty() &&(now - m_SpeedList.front().timetick > AVTIME*1000))
	{
		m_SpeedList.pop_front();
	}

	unsigned int span=now - m_SpeedList.front().timetick;
	if(span < 500) span=500; 
	unsigned int amount= m_nDownloadSum - m_SpeedList.front().amount;
	m_nAvDownSpeed= int(amount * (float(AVTIME*1000)/float(span)));

}


//the time that got bitfield
unsigned int CBTPeer::GetBitFieldTime()
{
	return m_nBitSetTimeTick;
}



//doWrite�ֽ��������
//tik for tat write
//��д�����Ҳ����ٶ����Ƶ�Լ��
//send command, don't limit by speedlimit
int CBTPeer::DoCmdWrite( int count , bool inlisten)
{


	if ( count <=0)
	{
		m_bCanWrite=false;
		maskWrite(false);
		return 0;
	}
	else if(!m_bCanWrite)
	{
		maskWrite(true);
		return 0;
	}

	int sendCount=0;

	//CheckRequest will send request to buffer
	CheckMyRequest();

	while ( !m_sendBuffer.empty() ) //�������������д��Ҫд��ȫд��
	{
		//when the network shutdown ,send will received a signal SIGPIPE in linux!
		int ret = send( GetHandle(), m_sendBuffer.data(), m_sendBuffer.size(), 0 );

		if ( ret <0 )
		{

#ifdef WIN32

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno == EWOULDBLOCK )
#endif
			{
				m_bCanWrite = false;
				maskWrite( true );
				return sendCount;
			}
			else
			{

				m_bCanWrite = false;
				m_CloseReason=CR_NETERR; 

				if(!m_bGotShakeFromPeer && m_bSendShakeToPeer)
				{//�õ�����ǰ���������رն��ǿ��ɵĹرգ��п�����ISP�Ͽ���������
					//OutMsg(L"shake neterror",MSG_ERROR);
					m_CloseReason=CR_SHAKE_NETERR;
				}

				OnClose();
				return sendCount;
			}

			break;
		}
		else if(ret==0)
		{
			//the connection is closed
			m_bCanWrite = false;
			m_CloseReason=CR_PEERCLOSE;

			if(!m_bSendShakeToPeer)
			{
				//OutMsg(L"no shake close");
				m_CloseReason=CR_NOSHAKE;
			}
			else if( !m_bGotShakeFromPeer)
			{
				m_CloseReason=CR_SHAKE;
			}

			OnClose();
			return sendCount;
		}
		else 
		{
			sendCount+=ret;
			m_sendBuffer.erase( 0, ret );
		}

	}


	if(inlisten)
	{
		//����ʱֻ�����������û�����ݿ�дʱҪ�������ѭ��
		//��Ϊ����ʱ�����٣�һ�㶼����д�꣬��������������д����ģ����洦����
		maskWrite(false);

	}

	return sendCount;

}

int CBTPeer::DoDataWrite(int count)
{

	if(!m_bUploadMode) {
		return DoBalenceWrite(count);
	} else {
		return DoEqualWrite(count); //�ϴ�ģʽ����������
	}

}


//write balence data, give the data that gave me the data
int CBTPeer::DoBalenceWrite(int count)
{

	//OutputDebugString(L"DoBalenceWrite\n");

	//2007/10/02 �¼Ӳ���
	if ( count <=0 )
	{
		m_bCanWrite=false;
		maskWrite(false);

		//OutputDebugString(L"DoBalenceWrite count<=0!!!\n");
		return 0;
	}
	else if(!m_bCanWrite)
	{
		maskWrite(true);
		//OutputDebugString(L"DoBalenceWrite !m_bCanWrite!!!\n");
		return 0;
	}


	int sendCount=0;


	//if the buffer already have left bytes, write it out before send piecedata
	while ( !m_sendBuffer.empty() )
	{
		//���������ȷ����������
		int canwrite=MIN(count-sendCount, int(m_sendBuffer.size()));

		if(canwrite<=0) {
			//OutputDebugString(L"DoBalenceWrite canwrite<=0!!!\n");
			break;
		}

		int ret = send( GetHandle(), m_sendBuffer.data(), canwrite, 0 );

		if ( ret <0 )
		{

#ifdef WIN32

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno == EINTR )
#endif
			{
				//OutputDebugString(L"DoBalenceWrite would block!!!\n");
				m_bCanWrite = false;
			}
			else
			{

				m_bCanWrite = false;
				m_CloseReason=CR_NETERR; 

				if(!m_bGotShakeFromPeer && m_bSendShakeToPeer)
				{//�õ�����ǰ���������رն��ǿ��ɵĹرգ��п�����ISP�Ͽ���������
					//OutMsg(L"shake neterror",MSG_ERROR);
					m_CloseReason=CR_SHAKE_NETERR;
				}

				OnClose();
				return sendCount;
			}

			break;
		}
		else if(ret==0)
		{
			//the connection is closed
			m_bCanWrite = false;
			m_CloseReason=CR_PEERCLOSE;

			//OutputDebugString(L"DoBalenceWrite connection closed!!!\n");

			if(!m_bSendShakeToPeer)
			{
				m_CloseReason=CR_NOSHAKE;
			}
			else if( !m_bGotShakeFromPeer)
			{
				m_CloseReason=CR_SHAKE;
			}

			OnClose();
			return sendCount;
		}
		else 
		{

			sendCount += ret;
			m_sendBuffer.erase( 0, ret );
		}

	}

	//all the buffers data is clear, send the piecedata if have
	//send the piecedata one by one! don't send many block in one circle!
	while( sendCount < count && m_bCanWrite && CheckPeerRequest() )
	{

		//���������ȷ����������
		int canwrite=MIN(count-sendCount, int(m_sendBuffer.size()));

		if(canwrite<=0) break;

		int ret = send( GetHandle(), m_sendBuffer.data(), canwrite, 0 );

		if ( ret <0 )
		{

#ifdef WIN32

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno == EINTR )
#endif
			{
				m_bCanWrite = false;
				maskWrite(true);
			}
			else
			{

				m_bCanWrite = false; 
				m_CloseReason=CR_NETERR; 

				if(!m_bGotShakeFromPeer && m_bSendShakeToPeer)
				{//�õ�����ǰ���������رն��ǿ��ɵĹرգ��п�����ISP�Ͽ���������
					//OutMsg(L"shake neterror",MSG_ERROR);
					m_CloseReason=CR_SHAKE_NETERR;
				}

				OnClose();
				return sendCount;
			}

			break;
		}
		else if(ret==0)
		{
			//the connection is closed
			m_bCanWrite = false;
			m_CloseReason=CR_PEERCLOSE;

			if(!m_bSendShakeToPeer)
			{
				m_CloseReason=CR_NOSHAKE;
			}
			else if( !m_bGotShakeFromPeer)
			{
				m_CloseReason=CR_SHAKE;
			}

			OnClose();
			return sendCount;
		}
		else 
		{

			sendCount += ret;
			m_sendBuffer.erase( 0, ret );
		}

	}


	if(m_bCanWrite)
	{
		maskWrite( false );
	}

	return sendCount;
}

//�������и�ԣʱ�����ƽ��������
//we have more bandwidth for tik-for-tat, so write piece equally to unchoked peer
//decredit: �Ƿ�һ����Ҫ�������ü����ϴ�ģʽ�����ͣ��ֵ�֮�䲻���ͣ���������
int CBTPeer::DoEqualWrite(int count, bool tictat)
{	

	if ( count <=0)
	{
		m_bCanWrite=false;
		maskWrite(false);
		return 0;
	}
	else if(!m_bCanWrite)
	{
		maskWrite(true);
		return 0;
	}



	int sendCount=0;

	//if the buffer already have left bytes, write it out before send piecedata
	while ( !m_sendBuffer.empty() && sendCount < count )
	{

		int canwrite=MIN(count-sendCount, int(m_sendBuffer.size()));
		if(canwrite<=0) break;

		int ret = send( GetHandle(), m_sendBuffer.data(), canwrite, 0 );

		if ( ret <0 )
		{

#ifdef WIN32

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno == EINTR )
#endif
			{
				m_bCanWrite = false;
			}
			else
			{
				m_bCanWrite = false;
				m_CloseReason=CR_NETERR; 
				OnClose();
				return sendCount;
			}

			break;
		}
		else if(ret==0)
		{
			//the connection is closed
			m_bCanWrite = false;
			m_CloseReason=CR_PEERCLOSE;
			OnClose();
			return sendCount;
		}
		else 
		{
			sendCount += ret;
			m_sendBuffer.erase( 0, ret );
		}

	}

	//equal write only send one block in a circle, so every peer will have 
	//the chance to get this extra block
	if( sendCount < count && m_bCanWrite && CheckPeerRequest(tictat)  )
	{

		int ret = send( GetHandle(), m_sendBuffer.data(), MIN(count-sendCount, int(m_sendBuffer.size())), 0 );

		if ( ret <0 )
		{

#ifdef WIN32

			if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else

			if ( errno == EAGAIN || errno == EINTR )
#endif
			{
				m_bCanWrite = false;
				maskWrite(true);
			}
			else
			{
				m_bCanWrite = false;
				m_CloseReason=CR_NETERR; 
				OnClose();
				return sendCount;
			}

		}
		else if(ret==0)
		{
			//the connection is closed
			m_bCanWrite = false;
			maskWrite(false);
			m_CloseReason=CR_PEERCLOSE;
			OnClose();
			return sendCount;
		}
		else 
		{
			sendCount += ret;
			m_sendBuffer.erase( 0, ret );
		}

	}

	if(m_bCanWrite)
	{
		maskWrite(false);
	}


	return sendCount;
}

//this functino is checked ok
//check the m_PeerId and set m_bIsSameAgent if the client is same as me
void CBTPeer::CheckAgent()
{
	//�ܹ�20���ֽ���������
	//[0-5]=�汾����ʶ����, ������17��18�����ֽ������
	//[6-15]�����[16] ��ǰ[6-15]�ֽڵ����� 
	//[17]�����������2λ��Զ��0����6λ��Զ��1
	//[18]�����������3λ��Զ��0����7λ��Զ��1
	//[19]��ǰ��19�ֽڵ�����
	//���������������ǿ��ܵ�ͬ�飬������ж�������ȷ���Ƿ���monma�ͻ���


	//0.���Ƕ��ǿ�����չ
	if(!m_bFastExtension) {
		return;
	}

	//1. �����У���
	char sum=m_PeerId[0];
	int i;
	for(i=1;i<19;i++)
	{
		sum^=m_PeerId[i];
	}

	if(sum!=m_PeerId[19]) {
		return;
	}

	//2�����17��18���ֽ��Ƿ�ϱ�
	char c17=m_PeerId[17];
	char c18=m_PeerId[18];

	if((c17 & 0x22) != 0x20 )  //0x20= 00100000b ,0x22= 00100010b
	{
		return;
	}

	if((c18 & 0x44)!= 0x40 )	//0x44= 01000100b, 0x40= 01000000b
	{
		return;
	}

	//3.���6-15λ�Ƿ����16λУ��

	sum=m_PeerId[6];

	for(i=7;i<=15;i++)
	{
		sum^=m_PeerId[i];
	}

	if(sum!=m_PeerId[16])
	{
		return;
	}

	//ok, all check pass
	m_bBrother=true;

#ifdef _CHECK
	//OutMsg(L"Found a brother", MSG_WARNNING);
	//PrintPeerInfo();
#endif
}

//�����Է����ã����ⲿ�ӿ�
void CBTPeer::AdjustCredit()
{

/*
	if(m_bBrother) m_nPeerCredit=128;
	else if(m_nDownloadSum>128) m_nPeerCredit=16;
	else if(m_nDownloadSum>64) m_nPeerCredit=12;
	else if(m_nDownloadSum>32) m_nPeerCredit=8;
	else if(m_nDownloadSum>16) m_nPeerCredit=4;
	else if(m_nDownloadSum>4) m_nPeerCredit=2;
	else m_nPeerCredit=1;
*/
}



//���ڼ����л����ӵĵ��ã���öԷ���infohash��Ӧ��������
void CBTPeer::SwitchAdmin(CPeerAdminBase *manager)
{

	assert(m_bAccepted);
	assert(!m_bIsA);
	assert(m_LinkStatus == LS_CONNOK);
	assert(manager!=NULL);

	SetDealer(NULL);


	m_ActiveCheckTimer=0;
	m_NewPieceNoticeCheckTimer=0;

	m_pParent->GetSpeedControl()->UnregisteClient(this);

	m_pParent=manager; //switch

	//�ָ���Ҫ������
	SetDealer( m_pParent->GetDealer() );

	m_pParent->GetSpeedControl()->RegisteClient(this);

	m_ActiveCheckTimer = AddTimer(60 * 1000 );

	m_NewPieceNoticeCheckTimer = AddTimer( 3000 );

	m_bTransfered=true;
}


//call by speed control
int CBTPeer::DoEqualWriteForDownloadMode(int count, bool force)
{
	if(m_bUploadMode) return 0;

	if(m_bBrother)
	{ //���ֵ�,�����ͶԷ����ã���Ϊ���������ö������
		return DoEqualWrite(count); 
	}

	if(!force) return 0;	//��ǿ���ϴ�

	return DoEqualWrite(count);

	/*
	else if(//m_bMeInterestPeer && 
			//m_bPeerInterestMe && 
			//m_nDownloadSum>0 && 
			m_PeerBitSet.GetPersent() < 0.5f)
	{
		return DoEqualWrite(count);
	}*/
	
/*
	else if( m_pParent->GetSession()->GetStorage()->GetPeerCredit(m_PeeriIP) >0)
	{
		return DoEqualWrite(count, true);
	}
*/

	return 0;
}

int CBTPeer::GetAvSpeed()
{
	return m_nAvDownSpeed;
}

int CBTPeer::GetDownloadSum()
{
	return m_nDownloadSum;
}

int CBTPeer::GetUploadSum()
{
	return m_nUploadSum;
}

std::string CBTPeer::GetPeerId()
{
	return m_PeerId;
}

//������չ
void CBTPeer::SendListenPort()
{

	if(m_bPortExchange && m_bBrother) 
	{
#ifdef _CHECK
		//OutMsg(L"send listen port with port exchange",MSG_INFO);
		//PrintPeerInfo();
#endif
		//listen port: <len=0x0003><op=0xE0><port>
		unsigned char buf[ 7 ] ;

		unsigned int len=3;
		len=htonl(len);
		memcpy(buf,&len,4);

		buf[4]= (0xE0);

		unsigned short iport= m_pParent->GetSession()->GetStorage()->GetListenPort();
		iport=htons(iport);

		memcpy(buf+5,&iport, 2);

		SendData( buf, 7 );

		m_LastMyActiveTick = GetTickCount();
		//return ;
	}	

/*
	if(!m_bAccepted && !m_bBrother && m_bUtPex)
	{
#ifdef _CHECK
		OutMsg(L"send listen port with utpex",MSG_INFO);
#endif
		//send ut_pex shake and give it my listen port
		//hope it will connect me

		CBenNode ben;
		ben.OpenDictionary();
		ben.AddValue("e",1);
		ben.AddValue(1);
		ben.AddValue("p",1);
		ben.AddValue(m_pParent->GetSession()->GetStorage()->GetListenPort());
		ben.AddValue("v",1);
		ben.AddValue("Benliud",7);
		ben.CloseDictionary();

		char buf[512]; //it's should enough
		buf[0]=0;	//shake id=0
		int pos=0;
		Encode(&ben, buf+1, pos);
		SendData(buf,pos+1);
		return;
	}
*/
}




bool CBTPeer::IsBrother()
{
	return m_bBrother;
}


unsigned int CBTPeer::GetPeeriIP()
{
	return m_PeeriIP;
}


CBTPiece& CBTPeer::GetPeerBitSet()
{
	return m_PeerBitSet;
}

//�ڹر����ǲ�����Ȥ������ʱ�������ȹر�û��allowfast���������
//������������Щ�������µ�allowfastƬ�����������̾ͱ��رա�
//bool CBTPeer::HaveGaveAllowFast()
//{
//	return !m_MeAllowFastList.empty();
//}

//������Ȥ������ҲӦ����һ���رյ����ȼ����ȹر����ȼ��͵ģ���ظߵ�
//����Ҳ�ᵼ��CPU���ߣ���Ϊ���ô���̫����
int CBTPeer::GetNotInterestPriority()
{
	return 
//		m_nDownloadSum > 150 ? 150: int(m_nDownloadSum)
		int(m_nDownloadSum)
//		+	m_nUploadSum > 50 ? 50: int(m_nUploadSum)
		+	m_MeAllowFastList.empty() ? 0 : 50
		+	m_bAccepted ? 30 : 0
		+	m_bBrother ? 20 : 0
		+	m_bFastExtension ? 10:0
		+	m_bEncryption ? 10:0 ;
}

//���ӳ��п�λ�ã������ӱ��ƶ������ӳأ�����Ƿ��𷽣����Է��������ˡ�

void CBTPeer::MoveToConnectedList()
{
	//�������ӽ���ʱ����shakehand,��Ϊ�����ᱩ¶���ǵ�PEERID
	//������ñ䶯��PEERID��������ᵼ�¶Է�����ķ������
	//��Ҫȷ�ϵ��ǣ����ӱ��������ӱ����ҵ��Լ���λ�ú��ٷ�������
	//�����������˵�ʱ�򣬷�����Ҳ��Ч�������������ر����ӵ��¶Է���������
	//����������Σ���������ʱ�����ӱ���������ӳ������Ϳ��Թر����������
	//���ӣ�ֻ�ǿ���ȷ�������ǳɹ��Ķ��ѡ�

	//���ﱻ�ƶ������ӳأ�ִ��ԭ����onconnected������ַ������


	if(!m_bAccepted)
	{
		assert(m_bIsA);
		if(!m_bEncryption)
		{
			SendHandshake();
		}
		else
		{
			SendPublicKey();
		}
	}

	maskRead(true);
}


//���ݳ�ʱ���
bool CBTPeer::DataTimeoutCheck(unsigned int now, unsigned int lap)
{
	if(m_LastPieceData!=0)
	{
		return (now - m_LastPieceData > lap) ;
	}
	else if(m_nBitSetTimeTick==0)
	{
		return false;
	}
	else
	{
		return (now - m_nBitSetTimeTick > lap);
	}

}

//�����Ƿ���Ҫ����Է��ļ���֧��������ȱʡΪ������
void CBTPeer::ReportEncryptable(bool report)
{
	m_bReportEncryptable=report;
}

#ifdef _CHECK
//void CBTPeer::OutMsg( const wchar_t * msg, _MSGTYPE type )
//{
//	if(m_pParent->GetSession()==NULL) return; //û��ת������ǰ������ǿգ�
//	m_pParent->GetSession() ->LogMsg( (wchar_t*)msg, type );
//}

void CBTPeer::PrintPeerInfo()
{
	std::string s=GetPeerInfo();
	//OutMsg(s.c_str());
}


void CBTPeer::PrintTimeLine()
{
	wchar_t buf[128];
#ifdef WIN32	
	swprintf(buf, L"now=%d, contime=%d, shktime=%d, bittime=%d,cr=%d", GetTickCount(),
		m_nConnectTimeTick, m_nShakeTimeTick, m_nBitSetTimeTick, m_CloseReason);
#else
	swprintf(buf, 128,L"now=%d, contime=%d, shktime=%d, bittime=%d,cr=%d", GetTickCount(),
		m_nConnectTimeTick, m_nShakeTimeTick, m_nBitSetTimeTick, m_CloseReason);

#endif
	//OutMsg(buf);
}


std::string CBTPeer::GetPeerInfo()
{
	std::string info;

	info=GetIpString(m_PeeriIP);
	info+="/";
	info+=GetPeerIdString();
	info+="/";
	info+=GetMyIdString();

	if(m_bAccepted)
	{
		info+="/accepted";
	}

	if(m_bTestRead)
	{
		info+="/read";
	}

	if(m_bGotShakeFromPeer)
	{
		info+="/gotshake";
	}

	if(m_bEncryption)
	{
		info+="/mse";
	}

	if(m_bFastExtension)
	{
		info+="/fast";
	}


	if(m_bGotBitSet)
	{
		if ( m_PeerBitSet.IsAllSet() )
		{
			info+="/seed";
		}
		else if ( m_PeerBitSet.IsEmpty() )
		{
			info+="/empty";
		}
		else
		{
			info+="/have";
		}
	}
	else
	{
		info+="/nobit";
	}

	if ( m_bMeChokePeer )
	{
		info+="/MCP";
	}
	else
	{
		info+="/!MCP";
	}

	if ( m_bPeerChokeMe )
	{
		info+="/PCM";
	}
	else
	{
		info+="/!PCM";
	}

	if ( m_bPeerInterestMe )
	{
		//wcscat( buf, L"/PeerInMe" );
		info+="/PIM";
	}
	else
	{
		//wcscat( buf, L"/!PeerInMe" );
		info+="/!PIM";
	}

	if ( m_bMeInterestPeer )
	{
		//wcscat( buf, L"/MeInPeer" );
		info+="/MIP";
	}
	else
	{
		//wcscat( buf, L"/!MeInPeer" );
		info+="/!MIP";
	}

	return info;
}

std::string CBTPeer::GetMyIdString()
{
	//get the first 8 charater, escape it
	if(m_pParent->GetSession()!=NULL)
		return Tools::EscapeToString( (unsigned char*)(m_pParent->GetSession()->GetMyID()), 10);
	else return "no_transfer";
}

std::string CBTPeer::GetPeerStr()
{
	char buf[ 64 ];
	sprintf( buf, "%s:%u", GetIpString(m_PeeriIP).c_str(), m_PeeriPort );
	return buf;
}
std::string CBTPeer::GetPeerIdString()
{
	if(!m_bGotShakeFromPeer) return "no_shake";

	return Tools::EscapeToString((unsigned char*)(&m_PeerId[0]),10);
}
//void CBTPeer::OutMsg(const char *msg, _MSGTYPE type)
//{
//	wchar_t buf[ 256 ];
//
//	Tools::UTF2UCS( (char*)msg, buf, 256 );
//
//	OutMsg(buf,type);
//}

std::string CBTPeer::GetIpString(unsigned int iip)
{
#ifdef WIN32
	const char *ret;
	ret = inet_ntoa( *((in_addr*)(&iip)) );
#else
	const char *ret;
	char ipbuf[ INET_ADDRSTRLEN ];
	ret = inet_ntop( AF_INET, (const void*)(&iip), ipbuf, INET_ADDRSTRLEN );
#endif

	return ret;
}
#endif



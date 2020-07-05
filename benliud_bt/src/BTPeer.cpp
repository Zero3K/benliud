/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


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

//这个值调到16似乎也没什么用
#define PENDING_REQUEST_SIZE	(12)

//the peer can request 16 piece data at onetime
#define PEER_REQUEST_SIZE	(24)	

//the connect timeout in second
//#define CONNECT_TIMEOUT		(10)	//现采用可变超时

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
	m_nDownloadPrority=0; //下载优先级
	m_nUploadPriority=0;  //上传优先级
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

//外部管理器关闭，可以带原因关闭
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
		
		SaveOrphanToStorage(); ////这里不稳定???

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
	{//因为流量限制所以不读，但监听读又可能引起高CPU负担
		//那么用户不限流后能及时把数据写出去吗？因为没有监听写许可
		m_bCanRead=false; //做这个标记则可以利用上面的处理在下次调用时恢复监听写许可
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

			this->OnClose();	//有崩溃问题！！！

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
				{//得到握手前的网络错误关闭都是可疑的关闭，有可能是ISP断开连接引起
					m_CloseReason=CR_SHAKE_NETERR;
				}

				OnClose();

				return readCount;
			}

		}
		else
		{//ret > 0


			readCount += ret;

			//加密过程完全结束后m_MSE_State == MSE_FINISH，这里才完全接管解密
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
				//一分内普通连接没完成握手,
				//部分原因是暂时没有处理48字节发起的握手
				m_CloseReason=CR_SHAKE_TIMEOUT;

				OnClose();
				return;
			}

		}
		else if( m_ActiveTimerCounter == 2 ) 
		{
			if(!m_bEncryption && !m_bGotBitSet )
			{//非加密连接没在2分内得到bitset
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
			{	//加密连接没在2分钟内完成握手
				m_CloseReason=CR_SHAKE_TIMEOUT;

				OnClose();
				return;

			}
		}
		else if( m_ActiveTimerCounter == 3 )
		{//加密连接没在3分内得到bitset
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

	memcpy(sbuf, sShake, 28); //范围[0,27]字节
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
	//有些愚蠢的客户端不能分辨我们发起的多连接，会在同一个连接上发两次bitset
	//我们如果不防备，就会也发两次，对方就会怀疑我们有问题了。

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

//加密过程完全结束后m_MSE_State==MSE_FINISH，此函数才接管加密
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

	maskWrite(true); //是否需要？要检验
}

int CBTPeer::ProcessData()
{
	//发起连接的加密处理过程
	if(!m_bAccepted && m_bEncryption && m_MSE_State!=MSE_FINISH)
	{//we are in DH secret progress block it to new process
		//我们主动发起加密连接且加密握手没结束
		//assert(m_bIsA);
		if(!DoDHSecretShake()) //if DoDHSecretShake return false, no continue do the later
		{
			return 0;
		}

	}

	//接受连接的加密处理过程
	if(m_bAccepted && !m_bGotShakeFromPeer)
	{
		assert(!m_bIsA);
		//accepted peer we don't send shake first,so this packet should be the shake of peer
		if(!m_bEncryption) //缺省是不加密的
		{
			if(!CheckAcceptedShakeHand()) //这里可能会设置加密状态m_bEncryption
			{
				return 0;
			}

		}
		else
		{//已经由CheckAcceptedShakeHand确认进入了加密状态，根据现在的加密进度，调用加密处理函数
			//或者我们只接受加密连接而设置了加密位m_bEncryption都到这里
			switch(m_MSE_State)
			{
			case MSE_INIT: //已经确认了加密m_bEncryption，但还没得到pubkey
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

	//握手包检查
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


			//可以继续往下执行包命令

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


	//到这里肯定是获得了握手包
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

		//包的长度校验，太长太短都可能不对
		// len 最小可以==1
		if( packlen > 4 + 8 + _CHUNK_LEN )
		{
			m_CloseReason=CR_BAD_DATA;
			OnClose();
			return 0;
		}

		if ( m_recvBuffer.size() >= ( packlen + 4 ) )
		{//完整命令

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

//调用这个函数可能有几种情况
//1。对方发起加密连接，在最后阶段调用这个函数校验HASH，这种情况无须转移，加密时已经转移连接
//2。对方发起普通连接不会调用这个。
//3。我方发起普通连接，对方反馈时调用这个检查，不转移连接
//4。我方发起加密连接，最后调用这个检查，不转移连接。
//所以这个函数里应该不用转移连接
bool CBTPeer::CheckHandshake( std::string info )
{

	assert(m_pParent->GetSession()!=NULL); //报告这里LINUX版有问题


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
	case 0xE0: //奔流扩展命令，对方的监听端口
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

		return -1; //保护m_pParent->GetSession()
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

		//假设就是得到了bitset. 以后再重复得到则取消先得到的哪个bitset

		m_PeerBitSet.Init(m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount() );
		m_PeerBitSet.Set(index, true);

		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, true );
		m_bGotBitSet=true;

		SendBitfield(); // ppc 补充代码
	}
	else
	{

		//有可能这个片已经存在，我们需要确保不重复向中心提交这个重复的东西。
		if(m_PeerBitSet.IsSet(index))
		{
			//重复的片，已经被设置了，不能向中心再次提交，否则中心的统计就不一致了
			return 0; //不用在后面判断，因为无变化
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

	if(!IsShaked()) return 0; //m_pParent->GetSession()都没有检查，如果没有发生转移前就出现这个包就完了

	int piecenum=m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount();
	int bytenum= (piecenum/8) + ((piecenum%8)?1:0);

	if(dataLen != (unsigned int)bytenum) {
		m_CloseReason=CR_BAD_DATA;
		OnClose();
		return 0;
	}


	if(m_bGotBitSet)
	{//重复的bitset,抛弃

#ifdef _CHECK
		//OutMsg(L"got bitset again!!",MSG_ERROR);
		//PrintPeerInfo();
#endif
		//取消老的bitset,保持数据一致
		assert(m_pParent->GetSession()!=NULL);
		assert(m_pParent->GetSession()->GetStorage()!=NULL);
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );

	}

	std::string bitset;
	bitset.append( ( const char* ) data, (unsigned int)bytenum);
	m_PeerBitSet.Init( bitset, piecenum );
	m_bGotBitSet=true;	

	m_nBitSetTimeTick=GetTickCount();

	//检查这个bitset是否是有伪造的回退痕迹
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
		//是接受进的连接，我们先不发BITSET，等收到对方的后再发
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
		//IsPieceInterest应该返回我们感兴趣的片的数量，这样好评价可交换量的参数！2007/09/07
		//在我们是选择性下载时，可以更准确的判断对方数据对我们的价值
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
	if (!IsShaked()) //保护m_pParent->GetSession()
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

	//检查请求是否越界
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
		//请求的是allowfast piece ,立即发出

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

		return 0; //已经发出数据，不必记录请求了，返回


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

	//UT和迅雷都很高，达到20个
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

	//检查是否已经完成任务

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

	if ( !m_bSendBitSet ) //保证先发送bitset, 后发have
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

		//这个判断不会准的，因为你不知道对方是否是选择下载，所以，如果对方不感兴趣咱们就关闭
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

	//检查请求是否越界
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
		//取消老的bitset记录，保持一致性
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
		//是接受进的连接，我们先不发BITSET，等收到对方的后再发
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
		//取消老的bitset记录，保持一致性
		m_pParent->GetSession()->GetStorage() ->PieceChangeNotice( m_PeerBitSet, false );

	}

	//no need to notice piece
	m_PeerBitSet.Init( m_pParent->GetSession() ->GetTorrentFile() ->GetPieceCount() );
	m_bGotBitSet=true;
	m_nBitSetTimeTick=GetTickCount();

	//检查这个bitset是否是有伪造的回退痕迹
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
		//是接受进的连接，我们先不发BITSET，等收到对方的后再发
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
			//如果对方总是拒绝，而我们总是取得相同的任务，那么就会导致快速循环
			//取任务，放弃任务。。。所以碰到拒绝的，最好是关闭连接
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

	//检查请求是否越界
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
	{//优先发普通请求，被choke时再考虑allowfast，普通请求优先suggest piece

		if ( !m_MyRequest.Empty() )
		{
			DoMyRequest();
			return;
		}

		if( m_bFastExtension ) //可能有suggest
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

					//虚文件数据处理
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

		//取普通任务，到这里是没有发现suggest piece 可用
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

				//虚文件数据处理
				unsigned int voff, vlen;
				if(m_pParent->GetSession()->GetStorage()->GetAffectRangeByVirtualFileInPiece(index, voff,vlen))
				{
					m_MyRequest.SetVirtualData(voff,vlen);
				}

				//check if have orphan data
				//合作任务我们不处理孤儿数据
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
	{//发allowfast请求，能到这里肯定是快速扩展的，上面禁止了其他情况
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

					//虚文件数据处理
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

//这个函数作用是替换CheckOrphanData
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

	if(m_MyRequest.IsCoorperate()) return; //合作任务所有数据存在于共享请求里，不会删除

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
		int base= int(1000* exp(-((percent-50.0)*(percent-50.0)/200.0))); //峰值1000
		m_nUploadPriority= base+ (m_bBrother?0:1000); 
		return m_nUploadPriority;

	}
}

//上传优先级
int CBTPeer::GetUploadPriority()
{
	return m_nUploadPriority;
}

int CBTPeer::CalculateDownloadPriority()
{
	CBTPiece bitset;

	bitset= m_pParent->GetSession()->GetBitSet();

	unsigned int pc=bitset.GetSize();

	//对方有我们需要的数据量比例，这里要考虑选择性下载情况，要过滤掉我们不感兴趣的数据再计算
	//否则将放大对方的优先性，我们不需要的数据就当他没有，不过处理比较复杂，暂不计算
	//float peersum=float(m_PeerBitSet-bitset)/pc; 	

	//新的方法考虑了我们是有选择的下载！
	unsigned int weneed=m_pParent->GetSession()->GetStorage()->IsPieceInterest(m_PeerBitSet);
	float peersum=float(weneed)/pc;

	float mysum= float(bitset-m_PeerBitSet)/pc; //我们有对方需要的数据量比例，这里假设对方全部下载，因为我们不知道对方是否选择性下载

	//float exchange= peersum*mysum*1000; //能互相交换的比例
	//exchange最大值是0.5*0.5*1000=250 变化范围[0-250]
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
		int(m_nAvDownSpeed)    //速度，这个是重要值，不封顶，越多越好[0-]
		+int( m_nDownloadSum > 64 ? 64:m_nDownloadSum) 	//总量，这个权值应该封顶，传的再多有个极限权值[0-10]
		//+int( m_nDownloadSum > 10 ? 10:m_nDownloadSum) //调小总量的值做测试
		//-int( m_bPeerChokeMe ? 0 : (now-m_nChokedTick)/1000 )  //阻塞时间可能容易被欺骗，应该用上一个接收数据时间来代替
		-int(choketime)
		+int( m_bAccepted? 10 : 0 ) 	//接收[0,10]
		+int( m_bEncryption? 4 : 0 ) 	//加密[0,4]
		+int( m_bFastExtension? 3 : 0 ) 	//快速协议支持[0,3]
		+int( peersum*200 )		//对方有我感兴趣的数据量
		+int( m_bBrother ? 10 : 0)	//兄弟
		+int( m_bSeed ? 100 : int(mysum * 100 * 0.2) );		//我们有对方需要的数据是一个次要量，[0-20] 有则更好，没有也罢
	//+( int(peersum * 100) > 20 ? 20 : int(peersum * 100) ) 	//对方有我们需要的数据的百分比[0-20]

	return m_nDownloadPrority;

}
//下载优先级用来优选下载端, choked 时间长度应在考虑中，所以记录上次被CHOKED的时间
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


	//一次处理最多发一个数据包

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

	//许多客户可能处理和我们不同，注销看看10/24
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
	//TODO: 同一个IP地址应该总返回相同的表，可以防止恶意客户端的多次欺骗！！

	//根据客户端的IP地址生成固定的序列，再到这个序列上去找我们的完成片，这样每次对
	//相同的IP请求都有相同的返回

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

	//希望是固定的位置不受欺骗, 可能这16个随即数都不能匹配到我们已经完成的块
	//不用理会，那说明我们没这个能力，因为我们自己都只有很少的数据。

	for(unsigned int i=0;i<16;i++)
	{
		//unsigned int r= *((unsigned int*)(iphash.data()+i));
		unsigned int r;
		memcpy(&r, iphash.data()+i, sizeof(unsigned int));

		//可以生成16个随即整数,利用这个随即数匹配我们有的片
		unsigned int rp=r%ps;	//匹配到合理的数据范围内

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

	//都是网络序
	m_pParent->GetSession()->GetStorage()->NewDHTNode(m_PeeriIP,iport);

	return 0;
}

//奔流扩展命令
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

	//对方监听任务的端口
	//都是网络序
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

//返回真，如果还有数据遗留需要老方式处理
//返回假，没有数据需要继续处理
bool CBTPeer::DoDHSecretShake()
{
	assert(m_bIsA);

	switch(m_MSE_State)
	{
	case MSE_INIT: //should not happen
		{
			//这里是对方在连接建立时立即发送了握手，我们都没有发起初始包，
			//那么，一般情况这都是普通握手，应该不是我们需要的加密握手反馈
			//对方应该没有加密连接的能力
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

		//我们发起的加密连接，对方如果不等待而直接发送普通握手，这里可能会收到68字节的握手
		//如果不处理的话，可能会傻等在这里！所以要判断，如果是这种情况，实际上对方不能支持加密连接

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

		//要等待处理
		//2 B->A: Diffie Hellman Yb, PadB	 [长度96-608] 
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

		MSE::BTDHTKey keya=MSE::EncryptionKey(true,m_MSE_DHSecret,infohash);  //A的加密密钥
		MSE::BTDHTKey keyb=MSE::EncryptionKey(false,m_MSE_DHSecret,infohash); //B的加密密钥

		m_MSE_pEncryptor=new MSE::RC4Encryptor(keyb,keya);//A的加密解密器

		m_MSE_pPeerEncryptor=new MSE::RC4Encryptor(keya,keyb); //B的加密解密器
		/*
		//The first 1024 bytes of the RC4 output are discarded. ???!!!!
		unsigned char discarded[1024];
		m_Encryptor->encryptReplace(discarded,1024); 
		m_Encryptor->decrypt(discarded,1024);

		m_MSE_pPeerEncryptor->encryptReplace(discarded,1024);
		m_MSE_pPeerEncryptor->decrypt(discarded,1024);
		*/

		//模拟对方计算ENCRYPT(VC),将来用于同步，
		//这个过程中我们是A方，我们不丢弃1K，B方应该能够识别我们没有丢弃1K
		//然后配合我们的加密方案给出正确的VC加密串，而不应丢弃1K后再加密
		//所以我们可以假设对方是不会丢弃1K的，如果他确实丢弃了，我们找不到VC。
		//说明我们和对方没有合拍，只有等他来连我们，因为我们可以识别他是否丢弃1K数据
		memset(m_MSE_ENCRYPTVC,0,8);
		m_MSE_pPeerEncryptor->encryptReplace(m_MSE_ENCRYPTVC,8);

		delete m_MSE_pPeerEncryptor;
		m_MSE_pPeerEncryptor=NULL;

		//得到了密钥，发送确认
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

		//如果要带握手包，则发ia=68, 握手包
		//否则发ia=0

		//这里我们发一个握手包


		//skip to len(IA), ignore PadC
		unsigned short ia=htons(68); //set len(IA) to 68

		memcpy(buf,&ia,2); 
		m_MSE_pEncryptor->encryptReplace(buf, 2);
		SendData(buf,2);  //发出len(IA)

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
	{//B的处理过程
		//这里处理第三步，找到同步点，发现对应的任务SKEY=infohash
		//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S)
		//HASH('req1',S)可以用来同步，而HASH('req2', SKEY)可以用来判断是对应哪个任务！
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
				m_MSE_REQ1POS=i+96; //绝对位置
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
			m_MSE_VCPOS=i+96; //绝对位置
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

//只有A需要检查ENCRYPT(VC)来同步
bool CBTPeer::MSE_AfterFoundVC()
{
	assert(m_bIsA);

	//check if ENCRYPT(VC, crypto_select, len(padD)) got
	//检查本次是否可以得到ENCRYPT(VC, crypto_select, len(padD))

	if(m_recvBuffer.size()-m_MSE_VCPOS < 8+4+2)
	{
		return false; //ENCRYPT(VC, crypto_select, len(padD)) 不完整
	}

	m_recvBuffer.erase(0, m_MSE_VCPOS); //now can safely remove the public key and the padB

	//the left is :
	//ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
	unsigned char swap[8+4+2];
	memcpy(swap,m_recvBuffer.data(),8+4+2); //copy VC+crypto_select,len(padD) 

	m_recvBuffer.erase(0, 8+4+2);	//对齐padd
	m_MSE_State=MSE_WAIT_PAD_D;

	m_MSE_pEncryptor->decrypt(swap,8+4+2);

	//unsigned int crypto_select=*((unsigned int*)(swap+8));
	unsigned int crypto_select;
	memcpy(&crypto_select, swap+8, sizeof(unsigned int));

	crypto_select=ntohl(crypto_select);

	//这里原先优选是不加密，后选加密，可能是这个原因和自己对接不好
	if(crypto_select & 1)
	{
		m_bFullEncryption=false; //以后不加密
	}
	else if(crypto_select & 2)
	{
		m_bFullEncryption=true; //以后加密
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

//只有连接发起者才等待处理这个函数
//4 B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
bool CBTPeer::MSE_AfterWaitPadD()
{
	assert(m_bIsA);

	if(m_MSE_PADDLEN==0)
	{

		if(m_bFullEncryption && !m_recvBuffer.empty())
		{

			m_MSE_State=MSE_FINISH; //加密握手过程结束

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
			m_MSE_State=MSE_FINISH; //加密握手过程结束
			return false;
		}
		else if(!m_bFullEncryption)
		{
			m_MSE_State=MSE_FINISH; //加密握手过程结束
			return !m_recvBuffer.empty();
		}

	}

	if(m_recvBuffer.size()<m_MSE_PADDLEN)
	{
		return false;
	}

	//解密PADD，然后丢弃padD，没有这个不能同步数据
	unsigned char abpadd[512];
	m_MSE_pEncryptor->decrypt(abpadd,m_MSE_PADDLEN);

	//删除PADD数据
	m_recvBuffer.erase(0,m_MSE_PADDLEN);


	//有没有剩余数据？
	if(m_recvBuffer.empty())
	{
		m_MSE_State=MSE_FINISH; //加密握手过程结束
		return false;
	}
	else
	{
		//		OutMsg(L"encrypt shake end,have payload stream");

		//剩余数据应该是ENCRYPT2(Payload Stream)

		if(m_bFullEncryption)
		{	
			//			OutMsg(L"padd is full encryption, decrypt it put back to buffer.");
			//解密移动到read后，这里需要解密剩余的部分
			m_MSE_State=MSE_FINISH; //加密握手过程结束

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
		{//不是全加密，剩余的都是没加密的

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
		//有些壳户端只发送48字节，不包含peer_id在内，按理也可以处理
		//但我们暂时等68字节来处理

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

		//单口监听时，这里获得了infohash, 要立即发送给上面通知，也许就要发生转移了。
		//转移后会重设m_pParent ,如果没有转移则可能GetSession()返回NULL,会错误

		//获得infohash, 立即转移，转移后parent就应改变了
		if(!m_pParent->GotHash(peerhash,this))  //这里会发生HASH校验
		{
			//HASH校验失败或无法接受连接
			m_CloseReason=CR_MYCLOSE;
			OnClose();
			return false;
		}


		m_PeerId = oldstyle.substr( 48, 20 );

		m_bGotShakeFromPeer = true;


		//如果没有设置m_pParent为volatile类型，这里可能会沿用上面老的指针
		//因为这个指针可能一直在寄存器里
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


		//我们是接受者，不先发送bitset

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


	//单口监听上必许在这里判断对应于哪个任务
	//前20字节HASH('req1', S)，后20字节HASH('req2', SKEY) xor HASH('req3', S)
	//后20字节用于判断连接属于哪个任务，此时m_MSE_pEncryptor==NULL

	//m_recvBuffer.erase(0, m_MSE_REQ1POS);
	//std::string hash1=m_recvBuffer.substr(0,20);
	//m_recvBuffer.erase(0,20);
	//std::string hash2=m_recvBuffer.substr(0,20);
	//m_recvBuffer.erase(0,20);

	m_recvBuffer.erase(0, m_MSE_REQ1POS+20);  //前20字节是HASH('req1',S) 不需要，在上一个函数里已经确认了
	std::string hashxor=m_recvBuffer.substr(0,20); //后20字节是HASH('req2', SKEY) xor HASH('req3', S)可用于判断任务归属
	m_recvBuffer.erase(0,20); //到这里删除了两个HASH，剩余的就是加密内容ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)

	//检查HASH('req2', SKEY) xor HASH('req3', S)来判断任务归属
	//我们不知道有哪些任务注册了，只有提交到listener来判断这个HASH会归属于哪个任务
	if(!m_pParent->GotEncryptHash( hashxor, m_MSE_DHSecret ,this))
	{//不是注册过的任务，关闭连接
		assert(m_LinkStatus==LS_CONNOK);

		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return false;
	}



	//现在多口监听和单口监听都移动到这里来处理m_MSE_pEncryptor了

	//如果GotEncryptHash返回真，则发生了管理转移，到了这里都是转移了的。

	//到这里已经发生了管理切换，可以计算m_MSE_pEncryptor了
	//因为切换过后就可以得到infohash
	MSE::BTDHTKey infohash(m_pParent->GetSession()->GetStorage()->GetTorrentFile()->GetInfoHash().data());

	MSE::BTDHTKey keya=MSE::EncryptionKey(true,m_MSE_DHSecret,infohash);  //A的加密密钥
	MSE::BTDHTKey keyb=MSE::EncryptionKey(false,m_MSE_DHSecret,infohash); //B的加密密钥

	m_MSE_pEncryptor=new MSE::RC4Encryptor(keya,keyb); //我们（B）的加密解密器,这回我们是B	

	unsigned char ency[8+4+2];
	memcpy(ency,m_recvBuffer.data(),8+4+2);

	//尝试不抛弃1K字节的解密，看看能不能得到全0的8字节VC
	unsigned char vc[8];
	memcpy(vc,ency,8);
	m_MSE_pEncryptor->decrypt(vc,8);

	for (int j=0;j<8;j++)
	{
		if(vc[j])
		{
			//不对，丢弃1K字节
			unsigned char discard[1024-8];
			m_MSE_pEncryptor->decrypt(discard,1024-8);
			m_MSE_Drop1K=true;
			break;
		}
	}

	if(m_MSE_Drop1K)
	{

		//再次解密
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
		{//还是解密失败，放弃

			m_CloseReason=CR_PROTOCOL;
			OnClose();
			return false;
		}
		else
		{
			//discard our 1024 bytes
			//成功，丢弃1K加密字节
			unsigned char dis[1024];
			m_MSE_pEncryptor->encryptReplace(dis,1024);
		}

	}
	else
	{
		//不需丢弃1K解密可以成功，继续解密剩余6字节
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
		m_bFullEncryption=false; //IA结束就不用解密了
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

//只有B才需要调用这个函数来等待PADC
//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
bool CBTPeer::MSE_AfterWaitPadC()
{
	assert(!m_bIsA);

	if(m_recvBuffer.size() < m_MSE_PADCLEN + 2) return false; //这里连同len(IA)一起等待

	unsigned char padc[512+2];
	memcpy(padc,m_recvBuffer.data(),m_MSE_PADCLEN+2);
	m_MSE_pEncryptor->decrypt(padc,m_MSE_PADCLEN+2);

	m_recvBuffer.erase(0,m_MSE_PADCLEN+2);	//padc 和 len(IA)一起删除

	unsigned short ialen;
	memcpy(&ialen,padc+m_MSE_PADCLEN,2);

	m_MSE_IALEN=ntohs(ialen);	//获得IA长度，最好是68，或者加上bitfield长度

	//由于程序暂时不能处理后面携带的bitset
	//（可能发生崩溃，因为发生管理转移后的处理还是由监听线程来做，bitset变化很复杂，曾经在这里抓住过崩溃），
	//如果IA!=0 and IA!=68直接关闭连接，避免用监听线程执行bitset变化操作
	//到这里都是发生了转移的，但执行线程仍是监听线程

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
	//为了稳定，直接关闭带bitset的IA连接，当发现不是这个原因导致的稳定问题时，可以取消这个操作
	if(m_MSE_IALEN!=0 && m_MSE_IALEN!=68)
	{
		m_CloseReason=CR_MYCLOSE;
		OnClose();
		return false; //不要让监听线程去执行bitset操作了。
	}

	//发送确认包，不带PAYLOAD，因为作为B首先要A先发IA。
	//下次获得了IA后我们才发送。这里我们不知道对方是否有IA过来
	//先发B->A: ENCRYPT(VC, crypto_select, len(padD), padD) 这部分
	//B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)

	unsigned char rep[8+4+2+512];
	memset(rep,0,8);	//vc

	unsigned int crypto_select= 2; //选择全加密方式

	if(!m_bFullEncryption) crypto_select=1; //以后不加密方式

	crypto_select=htonl(crypto_select);
	memcpy(rep+8,&crypto_select,4);

	unsigned short padd=(rand()%512);

	//设置padd随机
	for(unsigned short k=0;k<padd;k++)
	{
		rep[8+4+2+k]=rand()%0xFF;
	}

	unsigned int totallen=8+4+2+padd;

	padd=htons(padd);

	memcpy(rep+8+4,&padd,2);

	//加密数据
	m_MSE_pEncryptor->encryptReplace(rep,totallen);

	//发送加密数据，函数里不进行加密处理
	SendData(rep,totallen); 


	//下次等待IA
	m_MSE_State=MSE_WAIT_IA;
	return MSE_AfterWaitIA();


}

//只有B才需要调用这个函数来等待IA
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
		//全加密

		//解密数据返回
		size_t left=m_recvBuffer.size();

		unsigned char *ia=new unsigned char[left+2];
		memcpy(ia,m_recvBuffer.data(),left);

		m_MSE_pEncryptor->decrypt(ia,left);

		m_recvBuffer.resize(0);
		m_recvBuffer.append((char*)ia,left);

		m_MSE_State=MSE_FINISH;
		delete[] ia;

		//ia里可能有握手包和bitfield包，返回真可以直接后期处理，而且以后发送数据也是自动加密的了
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

	//对于单口监听，这里计算我们的解密器是时机不对，因为我们不知道这个连接对应的是哪个任务
	//一定要等对方指出想要连接的任务并且做了切换后才来计算这个m_MSE_pEncryptor
	//在这个步骤上我们只发公钥给对方
	//2 B->A: Diffie Hellman Yb, PadB	 [长度96-608] //B不能发送超过608字节, 30秒内也不少于96字节,否则应该关闭连接

	//3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S)
	//HASH('req1',S)可以用来同步，而HASH('req2', SKEY)可以用来判断是对应哪个任务！
	//S=m_MSE_DHSecret,SKEY=infohash
	//这些可以在下次的A->B反馈中来确定！这次确定不了的
	//下次A->B反馈中再构造m_MSE_pEncryptor


	unsigned char buf[96+512]; //max 512 byte pad
	unsigned int padb=((unsigned int)(rand()))%512;

	for(unsigned int i=0;i<padb;i++)
	{
		buf[96+i]=(unsigned char)(rand()%0xFF);
	}

	m_MSE_PublicKey.toBuffer(buf,96);

	SendData( buf, 96+padb);  //send DH public key, wait for B's public key

	m_MSE_State=MSE_SEND_PUB;

	//这个过程有个中断，不需要继续处理数据了
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
#define AVTIME	(20)	//平均速度计算时间

	//新法，记录一分钟的平均速度
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



//doWrite分解成三部分
//tik for tat write
//先写命令且不受速度限制的约束
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

	while ( !m_sendBuffer.empty() ) //命令本身不限制它写，要写就全写了
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
				{//得到握手前的网络错误关闭都是可疑的关闭，有可能是ISP断开连接引起
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
		//监听时只调这个，所以没有数据可写时要避免过度循环
		//因为监听时不限速，一般都可以写完，受网络条件限制写不完的，上面处理了
		maskWrite(false);

	}

	return sendCount;

}

int CBTPeer::DoDataWrite(int count)
{

	if(!m_bUploadMode) {
		return DoBalenceWrite(count);
	} else {
		return DoEqualWrite(count); //上传模式不降低信用
	}

}


//write balence data, give the data that gave me the data
int CBTPeer::DoBalenceWrite(int count)
{

	//OutputDebugString(L"DoBalenceWrite\n");

	//2007/10/02 新加测试
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
		//下两句更精确的限制流量
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
				{//得到握手前的网络错误关闭都是可疑的关闭，有可能是ISP断开连接引起
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

		//下两句更精确的限制流量
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
				{//得到握手前的网络错误关闭都是可疑的关闭，有可能是ISP断开连接引起
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

//当带宽有富裕时，大家平均都给点
//we have more bandwidth for tik-for-tat, so write piece equally to unchoked peer
//decredit: 是否发一个包要降低信用级别，上传模式不降低，兄弟之间不降低，其他降低
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
	//总共20个字节这样做：
	//[0-5]=版本及标识加密, 加密用17，18两个字节来异或
	//[6-15]随机，[16] 是前[6-15]字节的异或和 
	//[17]是随机，但第2位永远是0，第6位永远是1
	//[18]是随机，但第3位永远是0，第7位永远是1
	//[19]是前面19字节的异或和
	//满足以上条件的是可能的同伴，其后发送判断命令来确认是否是monma客户端


	//0.我们都是快速扩展
	if(!m_bFastExtension) {
		return;
	}

	//1. 检查总校验和
	char sum=m_PeerId[0];
	int i;
	for(i=1;i<19;i++)
	{
		sum^=m_PeerId[i];
	}

	if(sum!=m_PeerId[19]) {
		return;
	}

	//2。检查17，18两字节是否合标
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

	//3.检查6-15位是否符合16位校验

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

//调整对方信用，非外部接口
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



//单口监听切换连接的调用，获得对方的infohash后应立即发生
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

	//恢复必要的条件
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
	{ //帮兄弟,不降低对方信用，因为我们这是用多余带宽
		return DoEqualWrite(count); 
	}

	if(!force) return 0;	//非强迫上传

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

//奔流扩展
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

//在关闭我们不感兴趣的连接时，尽量先关闭没有allowfast输出的连接
//这样可以让有些起步者能下到allowfast片而不至于立刻就被关闭。
//bool CBTPeer::HaveGaveAllowFast()
//{
//	return !m_MeAllowFastList.empty();
//}

//不感兴趣的连接也应该有一个关闭的优先级，先关闭优先级低的，后关高的
//这里也会导致CPU升高，因为调用次数太多了
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

//连接池有空位置，此连接被移动到连接池，如果是发起方，可以发送握手了。

void CBTPeer::MoveToConnectedList()
{
	//不能连接建立时发送shakehand,因为那样会暴露我们的PEERID
	//如果采用变动的PEERID方案，则会导致对方更快的封闭我们
	//需要确认的是，连接必须在连接表里找到自己的位置后再发送握手
	//否则连接满了的时候，发握手也无效啊，反而立即关闭连接导致对方不信任你
	//所以无论如何，发送握手时，连接必须进入连接池里，否则就可以关闭这个建立的
	//连接，只是可以确认连接是成功的而已。

	//这里被移动到连接池，执行原来的onconnected里的握手发起操作


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


//数据超时检查
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

//设置是否需要报告对方的加密支持能力，缺省为不报告
void CBTPeer::ReportEncryptable(bool report)
{
	m_bReportEncryptable=report;
}

#ifdef _CHECK
//void CBTPeer::OutMsg( const wchar_t * msg, _MSGTYPE type )
//{
//	if(m_pParent->GetSession()==NULL) return; //没有转移连接前，这个是空！
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



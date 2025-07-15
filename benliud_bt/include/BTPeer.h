/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _BTPEER_H
#define _BTPEER_H

#include <string>
#include <list>
#include <vector>

/*
######################################################
### ----- Message Stream Encryption protocol ----- ###
### specification by Ludde/uau/The_8472/Parg/Nolar ###
######################################################

The following protocol describes a transparent wrapper for bidirectional
data streams (e.g. TCP transports) that prevents passive eavesdroping
and thus protocol or content identification.

It is also designed to provide limited protection against active MITM attacks
and portscanning by requiring a weak shared secret to complete the handshake.
You should note that the major design goal was payload and protocol obfuscation,
not peer authentication and data integrity verification. Thus it does not offer
protection against adversaries which already know the necessary data to establish
connections (that is IP/Port/Shared Secret/Payload protocol).

To minimize the load on systems that employ this protocol fast cryptographic
methods have been chosen over maximum-security algorithms.


----------------
- Declarations -
----------------

The entire handshake is in big-endian.
The crypto handshake is transparent to the next upper protocol,
thus the payload endianness doesn't matter.


A is the initiator of the underlying transport (e.g. a TCP connection)
B is the receiver

##### DH Parameters

Prime P is a 768 bit safe prime, "0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563"
Generator G is "2"

Xa and Xb are a variable size random integers. 
They are the private key for each side and have to be discarded after
the DH handshake is done. Minimum length is 128 bit. Anything beyond 180 bit
is not believed to add any further security and only increases the necessary
calculation time. You should use a length of 160bits whenever possible, lower
values may be used when CPU time is scarce.

Pubkey of A: Ya = (G^Xa) mod P
Pubkey of B: Yb = (G^Xb) mod P

DH secret: S = (Ya^Xb) mod P = (Yb^Xa) mod P

P, S, Ya and Yb are 768bits long

##### Constants/Variables


PadA, PadB: Random data with a random length of 0 to 512 bytes 

PadC, PadD: Arbitrary data with a length of 0 to 512 bytes, 

each can be used to extend the crypto handshake in future versions.
Current implementations may choose to set them to 0-length.
For padding-only usage in the current version they should be zeroed.


VC is a verification constant that is used to verify whether the other
side knows S and SKEY and thus defeats replay attacks of the SKEY hash.
As of this version VC is a String of 8 bytes set to 0x00.

crypto_provide and crypto_select are a 32bit bitfields.
As of now 0x01 means plaintext, 0x02 means RC4. (see Functions)
The remaining bits are reserved for future use.

The initiating peer A should provide all methods he supports in the bitfield,
but he may choose only to provide higher encryption levels e.g. if  plaintext
isn't sufficient for it's security needs.

The responding peer B should set a bit corresponding to the single method
which he selected from the provided ones.

Bits with an unknown meaning in crypto_provide and crypto_select
should be ignored as they might be used in future versions.


SKEY = Stream Identifier/Shared secret used to drop connections early if we
don't have a matching stream. It's additionally used to harden the protocol
against MITM attacks and portscanning.

Protocols w/o unique stream properties may use a constant.

Note: For BitTorrent, the SKEY should be the torrent info hash.



IA = initial payload data from A
may be 0-sized if you want to wait for the encryption negotation.

Peer A may buffer up to 65535 bytes before or during the DH handshake to append
it to the 3rd step. 
IA is considered as atomic and thus an implementation may
not expect that anything is handed to the upper layer before IA is completely
transmitted. Thus there must be no blocking operations within IA.


Note, Example for Bittorrent:
 After \19Bittorrent protocol + the BT handshake a block occurs since A waits
 for B to send his handshake before A continues to send his bitfield,
 thus IA can only include the prefix + the bt handshake but not the bitfield


###### Functions

len(X) specifies the length of X in 2 bytes.
Thus the maximum length that can be specified is 65535 bytes, this is
important for the IA block.


ENCRYPT() is RC4, that uses one of the following keys to send data:
"HASH('keyA', S, SKEY)" if you're A

"HASH('keyB', S, SKEY)" if you're B

The first 1024 bytes of the RC4 output are discarded.

consecutive calls to ENCRYPT() by one side continue the encryption
stream (no reinitialization, no keychange). They are only used to distinguish
semantically seperate content. 



ENCRYPT2() is the negotiated crypto method.
Current options are:
 0x01 Plaintext. After the specified length (see IA/IB) each side sends unencrypted payload
 0x02 RC4-128. The ENCRYPT() RC4 encoding is continued (no reinitialization, no keychange)


HASH() is SHA1 binary output (20 bytes)


###### The handshake "packet" format


The handshake is seperated into 5 blocking steps.

1 A->B: Diffie Hellman Ya, PadA  [����96-608] //A���ܷ��ͳ���608�ֽ�, 30����Ҳ������96�ֽ�,����Ӧ�ùر�����
2 B->A: Diffie Hellman Yb, PadB	 [����96-608] //B���ܷ��ͳ���608�ֽ�, 30����Ҳ������96�ֽ�,����Ӧ�ùر�����
A���� HASH('req1', S) ����ȷ����Կ�Ѿ��õ������HASH��B�����Լ������������B��Ҫ����A���͵�ǰ628�ֽڿ��Ƿ���
�����������У�PadA�ĳ��Ⱦ�ȷ���ˡ�
3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
Aͨ��ʶ��ENCRYPT(VC)������Ϳ���֪���������λ�ã���������Ϳ���ͬ���ˣ���ǰ�汾��VC��8���ֽڵ�0
4 B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
5 A->B: ENCRYPT2(Payload Stream)


Since the length of PadA and PadB are unknown
B will be able to resynchronize on HASH('req1', S)
A will be able to resynchronize on ENCRYPT(VC)


##### Optional early termination conditions
(should verified before the indicated step is started).

If a fail-fast behavior is prefered the following conditions can be used to
disconnect the peer immediately. If less recognizable patterns are preferred
a peer may wait and disconnect at a later point. If any of these conditions
are met the handshake can be considered as invalid.

2 (termination by B)
if A sent less than 96 Bytes within 30 seconds
if A sent more than 608 bytes 

3 (termination by A)
if B sent less than 96 Bytes within 30 seconds
if B sent more than 608 bytes

4 (termination by B)
if A didn't send the correct S hash within 628 bytes after the connection start (synchronisation point)
if A didn't send a supported SKEY hash after the S hash
if VC can't be decoded correctly after the SKEY hash
if none of the crypto_provide options are supported or the bitfield is zeroed
from here on it's up to the next protocol layer to terminate the connection

5 (termination by A)
if VC can't be decoded correctly within 516 bytes after the connection start (synchronisation point)
if the selected crypto method wasn't provided
from here on it's up to the next protocol layer to terminate the connection


Implementation Notes for BitTorrent Clients

Since not all BT clients will support this protocol there are 3 possible modes of operation:

   1. Support incoming connections with the Obfuscation Header and legacy BT headers but always try establish classic BT streams for outgoing connections

          This method ensures compatibility but only allows incoming obfuscated connections from traffic-shaped peers, obfuscated connections are never established to them, leading to a similar situation as if the shaped client was firewalled or NATed. 

   2. Support incoming connections for both protocols but try to establish connections with the Obfuscation Header first and retry with BT headers if that fails

          Compatibility is still ensured and allows obfuscated connections to shaped peers but might not be suitable for shaped peers and requires reconnect attempts when an obfuscated handshake fails for any reason. 

   3. Only support obfuscated streams and treat any incoming connection like a DH key exchange attempt.

          This method doesn't provide backwards compatibility but ensures obfuscation. The CPU load can still be adjusted by choosing and providing different encryption methods 

Any peer A in mode #1 or #2 should avoid unneccessary non-crypto connections to a peer B that uses crypto-only (mode #3) whenever possible.


[edit]
Tracker Extension

A client may append the following parameters to a HTTP GET:

    * supportcrypto=1 which means a peer can create and recieve crypto connections
    * requirecrypto=1 which means a peer only accepts/creates crypto connections
    * cryptoport=X in combination with port=0 which tells the tracker the port X on which the client is listening on as usual, but prevents the tracker from handing out a valid IP/Port combination if he doesn't support the crypto extension. This is only valid in combination with requirecrypto=1. cryptoport is not mandatory, a client may choose to use the port parameter as usual if it accepts legacy connections as a fallback measure. 

A tracker response when neither flag is set: The tracker will return IPs/Ports as usual but omit peers that signaled requirecrypto

A tracker response when the supportcrypto flag is set: In addition to the normal peers list (or string in the compact=1 case) an additional crypto_flags string which contains a bytearray in the same order as the peers list is sent. 1 means the corresponding peer requires crypto connections 0 means the peer supports and prefers classic BT

A tracker response when the requirecrypto flag is set: In addition to the normal peers list (or string in the compact=1 case) an additional crypto_flags string which contains a bytearray in the same order as the peers list is sent. 1 means the corresponding peer requires crypto connections 0 means the peer supports and prefers classic BT. The tracker should try to achieve the numwant limit with peers that flagged requirecrypto or supportcrypto first and only add non-supporting peers if no others are available.

Azureus supports this from 2403 B55+.
[edit]
Notes about the current Azureus implementation

Azureus tries to avoid unnecessary non-crypto connections through obfuscation-header aware Peer Exchanges, advanced messaging handshakes and Distributed tracker announces. Peer A tells its preferred connection mode in the handshake to peer B and then B adds additional flags to the PEX messages when it propagates A's address/port. This does not work in combination with Secure Torrents since they disable PEX and the DHT.

Azureus currently allows its users either to specify fallback support for incoming and outgoing connections seperately when the encryption is enabled or passive support only instead. Specified encryption methods aren't preferences but minimum requirements, for both incoming and outgoing connections, i.e. if a peer doesn't support the minimum (or possibly higher levels in the future) then the connection will can't be established. And Azureus will only offer minimum or higher level methods. 
*/

//#include "../../benliud/include/msgtype_def.h"
#include <Tools.h>

#include <Mutex.h>
#include <AutoLock.h>
#include <SockProxyTCPClientSock.h>
#include "BTPiece.h"

#include "SingleRequest.h"  

//MSE
#include "MSE_BigInt.h"
#include "MSE_Rc4encryptor.h"
#include "TTrackerDefines.h"	// Added by ClassView


enum TLinkState
{
    LS_INIT,
	LS_CONN,		//connecting 
	LS_CONNOK,		//connect ok
	LS_CLOSE,		//lost connection

};

//��������ιرյķǳ���Ҫ��
//�����ж��Ƿ���Ҫ�ü������ӷ�ʽ
enum TCloseReason
{
	CR_LINKFAIL=0,		//����ʧ��
	CR_NOSHAKE=1,			//�������Ͼ��������Է������رգ�������������ǶԷ����ӱ��ͻ�ܾ����ӻ�û���������
	CR_SHAKE_TIMEOUT=2,	//���ӳɹ���Ҳδ���Է������رգ���û�յ�����
	CR_SHAKE=3,			//���������ֵ��Է������ر�����
	CR_SHAKE_NETERR=4,	//���������ֺ�������������жϺ�رգ���������ܿ��ɣ�Ӧ���Ȳ��ü�������
	CR_BAD_SHAKE=5,		//���ǵõ��˴���������źţ������ر�����
	CR_BIT_TIMEOUT=6,		//����bitset��ʱ
	CR_NOT_ACTIVE=7,		//�Է�û�м�����������������������ر�����
	CR_DATA_TIMEOUT=8,	//�����ڵȴ������г�ʱ
	CR_BAD_DATA=9,		//����У�����
	CR_NETERR=10,			//������ʱ��������µ���������µĹر�����
	CR_PEERCLOSE=11,		//�Է����������عر�����
	CR_MYCLOSE=12,			//���������ر����ӣ���������ɣ��Է�Ҳ��ɣ�����Ҫ������
	CR_PROTOCOL=13,		//Э�����ر�����
	CR_SELF=14,			//�������Լ�
	CR_NOTINTEREST=15,	//������Ȥ�ر�
	CR_PICKCLOSE=16,	//ѡ���Թر�, ˵���Է��������ȼ��������ⲻ����
	CR_HONEST=17,		//�Է�������ƭ
	CR_NOROOM=18,		//����������û�пռ��������
	CR_NOENCRYPT=19,
} ;



class CPeerAdminBase;
class RC4Encryptor;
class CBenNode;
class CBTPeer : public SockLib::CSockProxyTCPClientSock
{
	class TPeerBlockRequest
	{
		public:
		unsigned int index;
		unsigned int offset;
		unsigned int len;
	};	

	class TReceivedDataRecord
	{
		public:
		unsigned int timetick;
		unsigned int amount;
	};
	
	enum MSE_State
	{
		MSE_INIT,
		MSE_SEND_PUB,
		MSE_GOT_PUB,
		MSE_FOUND_VC,    //for A
		MSE_WAIT_PAD_D,  //for A
		MSE_FOUND_REQ1,  //for B
		MSE_WAIT_PAD_C,  //for B
		MSE_WAIT_IA,	 //for B and B
		MSE_FINISH,

	};


	typedef std::list<TReceivedDataRecord> TSpeedRecord; //��¼�����ֵ������ٶ������Լ�����������ֵ��ٶȡ�
	typedef std::list<int> TAllowFastList;
	typedef std::list<int> TSuggestList;
	typedef std::list<TPeerBlockRequest> TPeerPieceRequestList;
	typedef std::vector<int> TNewPieceNoticeList;

public:
    CBTPeer(CPeerAdminBase * manager, bool uploadmode=false);
    virtual ~CBTPeer();

private:


    CPeerAdminBase*	m_pParent; //�������valatileҲû�ð���ȥ���ٹ۲�

	TLinkState		m_LinkStatus;

	TCloseReason	m_CloseReason;


	unsigned int	m_PeeriIP;		//�Է���ַ��������
	unsigned int	m_PeeriPort;	//�Է��˿ڣ�������

    unsigned int	m_ActiveCheckTimer;		//�����ʱ
    unsigned int	m_NewPieceNoticeCheckTimer;	//the timer that check new got piece and send have cmd

	unsigned int	m_ActiveTimerCounter;	//count the activetimer
	unsigned int	m_nConnectTimeTick;		//the time that connected
	unsigned int	m_nShakeTimeTick;		//the time that got shakehand
	unsigned int	m_nBitSetTimeTick;		//the time that got bitfield
    unsigned int	m_LastPeerActiveTick;	//for check the active
    unsigned int	m_LastMyActiveTick;		//record our send
    unsigned int	m_LastPieceData;		//record the time we last got data


	bool			m_bGotBitSet;			//if we got the peer's bitset?
	bool			m_bSendBitSet;
	bool			m_bGotHashFromPeer;		//��Щ����û��20�ֽ�peer_id,ֻ���ȷ�HASH
    bool			m_bGotShakeFromPeer;	//�õ���HASH��PEER_id
	bool			m_bSendShakeToPeer;		//�Ƿ����Ƿ��������ְ����Է�
    bool			m_bMeChokePeer;
    bool			m_bMeInterestPeer;
    bool			m_bPeerChokeMe;
    bool			m_bPeerInterestMe;
    bool			m_bFastExtension;
	bool			m_bPortExchange;		//�����˿ڽ�����������չ
    bool			m_bCanRead;
    bool			m_bCanWrite;
	bool			m_bSeed;
	bool			m_bBrother;				//�ֵܿͻ��ˣ�
    bool			m_bAccepted;			//is this link is accepted
	bool			m_bUploadMode;			//we have finish the job ,only for upload now
	bool			m_bTransfered;			//�Ѿ�������ת�Ƶı��
	bool			m_bUtPex;				//is peer support ut_pex extersion?
	bool			m_bReportEncryptable;	//�Ƿ񱨸�Է��ļ���֧���������ϲ�

	bool			m_bTestRead;			//������
	unsigned int	m_nTestReadBytes;		//������

	CSingleRequest	m_MyRequest;	

    TPeerPieceRequestList m_PeerRequestList;

    std::string		m_sendBuffer;
    std::string		m_recvBuffer;
	

	std::string		m_PeerId;

    CBTPiece		m_PeerBitSet;

    TAllowFastList	m_AllowFastList;	//the list we allowed by peer to fast download
    TAllowFastList	m_MeAllowFastList; //the list we allow the peer to fast download
    TSuggestList	m_SuggestList;
	TSpeedRecord	m_SpeedList;	//���ڼ�����������ֵ��ٶ�

    SockLib::CMutex			m_NewPieceNoticeListMutex;
    TNewPieceNoticeList	m_NewPieceNoticeList;


	int m_nDownloadSum;	//��������
	int m_nUploadSum;	//�ϴ�����
//	int m_nPeerCredit;	//���÷֣����÷ָ����Ǿͻ��͸��಻ƽ�����ݸ��Է���
	int m_nAvDownSpeed;	//���һ��ƽ��������
	int m_nDownloadPrority; //�������ȼ�
	int m_nUploadPriority;  //�ϴ����ȼ�

	unsigned int m_nChokedTick;	//���Ǳ��Է�������ʱ���

//MSE content
	bool m_bEncryption; //
	bool m_bIsA;		//are we at A side?
	bool m_bGotDHSecret;
	bool m_bFullEncryption;
	bool m_MSE_Drop1K;	//if drop the first 1k, default is not

	unsigned int m_MSE_VCPOS;
	unsigned int m_MSE_REQ1POS;
	unsigned int m_MSE_PADDLEN;
	unsigned int m_MSE_PADCLEN;
	unsigned int m_MSE_IALEN;

	MSE_State	m_MSE_State;
	MSE::BigInt m_MSE_PublicKey;  
	MSE::BigInt m_MSE_PrivateKey;
	MSE::BigInt m_MSE_DHSecret;
	MSE::RC4Encryptor* m_MSE_pEncryptor; //���ǵļ��ܽ�����
	MSE::RC4Encryptor* m_MSE_pPeerEncryptor; //�Է��Ľ��ܼ�����

	unsigned char m_MSE_ENCRYPTVC[8];//����A����B��ͬ����
	unsigned char m_MSE_REQ1HASH[20];//����B����A��ͬ����

private:
    void SendData( const void* data, size_t len );
    void SendHandshake();
	void MakeShake(char* sbuf);
    void SendBitfield();
    void sendInterested( bool interested );
    void SendHave( int index );
    void SendRequest( int index, unsigned int offset, unsigned int len );
    void SendPieceData( int index, unsigned int offset, std::string& data );
    void SendPieceCancel( int index, unsigned int offset, unsigned int len );
    void SendKeepAlive();
    void DoMyRequest();
    void SendHaveAll();
    void SendHaveNone();

    void SaveOrphanToStorage();
	void AdjustPeerPriorityForAgent();
	int DoEqualWrite(int count,bool tictat=false);	//call by DoDataWrite
	int DoBalenceWrite(int count);//call by DoDataWrite

public:
	void ReportEncryptable(bool report=false);
	void MoveToConnectedList();
	int GetNotInterestPriority();
	bool DataTimeoutCheck(unsigned int now, unsigned int lap);

	CBTPiece& GetPeerBitSet();
	void Attach( int handle, unsigned int iip, unsigned short iport);
	unsigned int GetPeeriIP();
	
	bool IsBrother();
	bool MeInterestPeer();
	bool PeerChokeMe();


	int GetUploadSum();
	int GetDownloadSum();
	int GetAvSpeed();
	int DoEqualWriteForDownloadMode(int count, bool force=false);

	std::string GetPeerId();


	void SwitchAdmin(CPeerAdminBase* manager);
	int DoCmdWrite(int count, bool inlisten=false);
	int DoDataWrite(int count);

	unsigned int GetBitFieldTime();

	void JobEndClose();
	void SetUploadMode(bool upload);
	bool IsShaked();
	unsigned int GetShakeTime();
	bool IsChokedByPeer();
	bool IsMeInterestPeer();
	bool IsGotBitSet();
	bool MSE_AfterConfirmEncrypt();
	bool MSE_AfterWaitIA();
	bool IsEncrypt();
	bool IsSeed();
	void SetEncrypt(bool encrypt, bool isA);

	TCloseReason GetCloseReason();

    virtual void OnRead();
    virtual void OnWrite();
    virtual void OnClose();
	virtual void OnConnectFail();
	virtual void OnConnectOk();
	virtual bool Connect( unsigned int iip, unsigned short iport, unsigned int timeout);

	int DoRead(int count );

    virtual void OnTimer( unsigned int id );

    TLinkState GetLinkState();


    bool IsAccepted();
    void ClosePeer(TCloseReason reason=CR_MYCLOSE);
    void BroadcastNewPiece( int index );

	int GetUploadPriority();
	int GetDownloadPriority();
    bool MeChokePeer();
    bool PeerInterestMe();
    void ChokePeer( bool chocke );
    void DownloadFinish(bool comp=true);
	int CalculateDownloadPriority();
	int CalculateUploadPriority();

#ifdef _CHECK
    void PrintPeerInfo();
	void PrintTimeLine();
	std::string GetPeerStr();
	std::string GetMyIdString();
	std::string GetPeerIdString();
	std::string GetPeerInfo();
	std::string GetIpString(unsigned int iip);
#endif
protected:
	bool CheckOutOrphanData();
	void SendListenPort();
	void AdjustCredit();
	void CheckAgent();
	void CalculateSpeed();
	int  DoPexCommand(void* data, size_t datalen);
	bool MSE_AfterWaitPadC();
	bool MSE_AfterFoundReq1();
	bool CheckAcceptedShakeHand();
	bool MSE_AfterWaitPadD();
	bool MSE_AfterFoundVC();
	bool MSE_AfterGotPub();
	bool MSE_AfterSendPub();
	bool DoDHSecretShake();
	int  DoCmdDHTPort( void * data, size_t dataLen );
	int  DoCmdPort( void * data, size_t dataLen );

	void CancelMyRequest();
	void CancelMyRequest( int index );
	void SendAllowFast( int index );
    void GenAllowFastPieceList();
    bool IsMeAllowFastPiece( int index );
    //void OnConnect();
    void OnConnectionClose();
	//void OnConnectFailed();
    void SendChoke();
    void SendUnChoke();
    void SendRejectRequest( int index, unsigned int offset, unsigned int length );
    bool CheckPeerRequest(bool tikfortat=true);
    void NoticeNewHavePiece();

    void CheckMyRequest();
    bool CheckHandshake( std::string info );
    int DoCmdAllowFast( void* data, int dataLen );
    int DoCmdRejectRequest( void* data, int dataLen );
    int DoCmdHaveNone( void* data, int dataLen );
    int DoCmdHaveAll( void* data, int dataLen );
    int DoCmdSuggestPiece( void* data, int dataLen );
    int DoCmd( unsigned char cmd, void* data, size_t dataLen );
    int DoCmdChoke( void* data, size_t dataLen );
    int DoCmdUnchoke( void* data, size_t dataLen );
    int DoCmdInterested( void* data, size_t dataLen );
    int DoCmdNotInterested( void* data, size_t dataLen );
    int DoCmdHave( void* data, size_t dataLen );
    int DoCmdBitfield( void* data, size_t dataLen );
    int DoCmdRequest( void* data, size_t dataLen );
    int DoCmdPiece( void* data, size_t dataLen );
    int DoCmdCancel( void* data, size_t dataLen );
    int ProcessData();
	void SendPublicKey();


};

#endif

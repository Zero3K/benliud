
#ifndef _STORAGE_H
#define _STORAGE_H

#include <string>
#include <list>
#include <set>
#include <stdio.h>
#include <map>

#include "datatype_def.h"
#include "../../benliud/bittorrent_types.h"

#include "BTStorage.h"
#include "BTPiece.h"
#include "BTPieceSum.h"
#include "ShareRequest.h"


#include <TFileInfo.h>
#include <TorrentFile.h>
#include <ThreadBase.h>
#include <Mutex.h>

#include "btpriority_def.h"
#include "TStorageFileInfo.h"
#include "TPieceCache.h"
#include "PeerCenter.h"


//#define REQUEST_BLOCK_SIZE (16*1024)


//���������죬����̶���128KΪ���ȣ�������Ƭ�ĳ���������
//��������ĵ����ڶԴ�Ƭ���Ŀ����ʱ��Ч
//������10M���棬����Դ��80����
//������Ƚ��ȳ����㷨�ųɶ���
//�Ӷ�ͷ�������ݿ飬�Ӷ�βѹ�����ݿ飬����ĳ�����ݿ鱻����ʱ�������β
#define BUFFER_BLOCK_SIZE (128*1024)

struct TBufferBlock
{
	unsigned int pIndex;	//Ƭ������
	unsigned int offset;	//Ƭ��ƫ��
	std::string  data;		//����=BUFFER_BLOCK_SIZE������
};  //��������BUFFER_BLOCK_SIZE



struct TVirtualAffect
{
	unsigned int offset;
	unsigned int length;
}; //���ļ���Ƭ��Ӱ�췶Χ


class CPieceRequest;

class CBTSession;

class CTorrentFile;

class CBTJob;

class CTrackerCenter;

class CBTListener;

class CBTStorage : public SockLib::CThreadBase //, public CAutoFileManager //the thread is for listener
{

	typedef std::list<CBTSession*> TSessionList;
	typedef std::list<int>	TIntList;
	typedef std::map<unsigned int, TVirtualAffect> TVirtualMap;
	//typedef std::vector<CPieceRequest*> TShareRequestList;
	typedef std::map<int, CShareRequest> TShareRequestMap;

	typedef std::map<unsigned int, std::string> TPieceMap;
	typedef std::list<TPriority> TPriorityList;

	typedef std::map<int, int> TDownloadingMap;
	typedef std::list<TBufferBlock> TReadBufferList;


	typedef std::list<COrphan> TOrphanList;
	typedef std::map<int, TOrphanList> TOrphanMap;

public:
    CBTStorage();
    virtual ~CBTStorage();


private:

    unsigned int	m_nCacheSize;
	unsigned int	m_nConnectingLinkMax;
	unsigned int	m_nConnectionLinkMax;
	unsigned int	m_nWasteByte;	//waste data amount
	unsigned int	m_nLastTickForCalSpeed; //last tick count we cal ave speed
	int				m_nTimeToFinish;  //cal by averagespeed and left piece
	int				m_nAverageSpeed;  //average download speed in bytes/s
	int				m_nAverageUpSpeed;
    int				m_nReadHit;
    int				m_nReadCacheHit;
	unsigned int	m_SessionIdCounter;
	int				m_nConnectingCounter;
	int				m_nListenSocket;
	int				m_UpByteCount;
	int				m_UploadSpeedLimit;
	int				m_DownByteCount;
	int				m_DownloadSpeedLimit;
	int				m_nTaskId;
	unsigned short	m_nListenPort;
	unsigned short	m_nDHTPort;
    bool			m_bOldFileExists;

    bool			m_bPieceSumChanged;
    bool			m_bPriorityChanged;
    bool			m_bIsTaskFinished;
	bool			m_bStop;

    llong			m_nBanedSize;
    llong			m_nSumOfDownload;
    llong			m_nSumOfUpload;

	BencodeLib::CTorrentFile*	m_pTorrentFile;
    CBTJob*			m_pParent;

	CTrackerCenter* m_pTrackerCenter;
	CBTListener*	m_pSingleListener;	//���˿ڼ����豸��ַ��ȫ������Ҳ������


	char			m_MyId[20*SESSIONNUM];

    float			m_fPieceAvailability;
    float			m_fFinishedPercent;
	float			m_fConnectingRatio;
	float			m_fConnectionRatio;


    SockLib::CMutex			m_FileHandleListMutex;
    SockLib::CMutex			m_bitPieceSumMutex;
	SockLib::CMutex			m_DownloadingMapMutex;
	SockLib::CMutex			m_ShareRequestMapMutex;
    SockLib::CMutex			m_ReadCacheMutex;
    SockLib::CMutex			m_WriteCacheMutex;
	SockLib::CMutex			m_EdgeCacheMutex;
	SockLib::CMutex			m_OrphanMapMutex;
	SockLib::CMutex			m_SessionListMutex;
	SockLib::CMutex			m_FileSystemLock; 



    TFileHandleList m_FileHandleList;

    CBTPiece		m_FinishedBitSet;
    CBTPieceSum		m_PriorityBitSet; // 0 indicate baned piece, other is priority
    CBTPieceSum		m_bitPieceSum; //all peer's bitset sum up
	std::string		m_InitBitSet;	//initial bitset get from db
	std::string		m_FilePriority;
    //the availabity based on peer's bitset and our choose of file
    //update it when : peer's bitset change or baned files changed.

	TDownloadingMap		m_DownloadingMap;

	TShareRequestMap	m_ShareRequestMap;
	TReadBufferList		m_ReadBufferList;
    TPieceMap			m_WriteCache;	//��VECTOR�Ϳ��ԣ�Ҫ����
	TPieceMap			m_EdgeCache;	//edge piece always store in memory
    TPriorityList		m_PriorityList;  //for baned file and for priority


	TOrphanMap			m_OrphanMap;

    TSessionList		m_SessionList;
	TIntList			m_SpeedHistoryList; //one second ,one data
	TIntList			m_UpSpeedHistoryList; //one second ,one data  
	TVirtualMap			m_PieceInVirtualFile;

	CPeerCenter			m_PeerCenter;		//����������

    UINT		m_CodePage; //for torrent file's encode if don't have utf8 fmt

    wchar_t		m_sDestPath[ 512 ]; //where to save

    wchar_t		m_sIndexPath[ 512 ]; //the index

	_JOB_STATUS m_JobStatus; //����״̬
	//�ڹ��������ʱ�Ͱ����ļ���Ƭ��Ӱ�����������Ժ�������
	//ǰ����Ƭ�ı�ţ�������Ƭ��Ӱ�췶Χ

private:
    bool OpenSingleFile( TStorageFileInfo& info );
    bool RebuildFileListStructure();
    bool OpenFiles();
    bool IsNeedCheckFiles();
    void CheckOldFiles();
    bool GetFileInfoByOffset(TStorageFileInfo& info, llong offset );
    unsigned int GetReadCacheSize();
    int WritePieceToDisk( int index, std::string& data );
    bool ReadDataFromDisk( std::string& data, int index, llong offset, unsigned int len );
	bool ReadDataWithoutBuffer( std::string& data, int index, unsigned int offset, unsigned int len );
	bool ReadPieceWithoutBuffer( std::string& data, int index );
    void GenPriorityBitSet();
    int GetPieceIndexByOffset( llong offset );
    void CalculateAvailability();
    void BuildFileListStructure();
	void JobInit();
	void CleanOrphan(int index);
	void InitFileProgress();
	void RecordNewPieceInFile(int index);
	llong PieceDistanceFromLastWrite(int index);
	void RecordLastWriteInFile(int index);
	bool ReadDataFromCache(std::string& data ,int index, unsigned int offset, unsigned int len);
    bool ReadPiece( std::string& data, int index );
	void MakeSureDirectoryExistsForFile(wchar_t* filePath);
	bool Convert(const char* multibyte, int nbytes, UINT codepage, wchar_t* wpBuf, int wbuflen);
public:
	void SetCodePage(UINT codepage) {m_CodePage=codepage;}
	int GetPeerCredit(unsigned int iip);
	void PeerSupportEncryption(unsigned int iip, bool enc);
	bool AnyUnCheckedNode();
	void LinkOkButPeerClose(unsigned int sessid, unsigned int iip);

	void NoticeFileProgress();
	bool SubmitShareData(unsigned int source, int index, unsigned int offset, std::string& data);
	bool GetShareTask(int index, unsigned int &offset,unsigned int& length);
	llong GetUnFinishedBytes();
	llong GetFinishedBytes();
	void GiveUpAcceptPeerLink( unsigned int sessid, unsigned int iip);
	void NewBenliudPeer(unsigned int iip, unsigned short iport);
	bool GetAffectRangeByVirtualFileInPiece( int index, unsigned int& offset, unsigned int& len);
	void GetLinkStatus(unsigned int& mylink, unsigned int& peerlink, unsigned int& trylink, unsigned int& total);
	void CloseReport( unsigned int sessid, unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset);
	void LinkReport( unsigned int sessid, unsigned int iip, bool ok);
	bool TryAcceptPeerLink( unsigned int sessid, unsigned int iip);

	bool GetPeerInfoToLink( 
		unsigned int sessid, 
		bool connectable, 
		unsigned int& iip, 
		unsigned short& iport, 
		int& encref, 
		unsigned int& timeout);

	void GiveUpLink( unsigned int sessid, unsigned int iip);
	int CheckBitSet( unsigned int sessid, std::string& peerid, unsigned int iip, CBTPiece& bitset);
	void LinkOkButNoRoomClose( unsigned int sessid, unsigned int iip);

	const char* GetMyID(unsigned int sessionid);
	bool IsAvialbilityChanged();


	unsigned int GetPieceCount();
	std::string GetPieceHash(int index);
	void SetSingleListener(CBTListener* listener);
	bool TransferPeer(CBTPeer* peer);
	void SetConnectionRatio(float ratio);
	void SetConnectingRatio(float ratio);

	void SetInitBitSet(std::string& bitset);
	bool ReadTorrentContent(std::string& content,  UINT codepage);

	int RunOffDownBytes(int bytes);
	int RunOffUpBytes(int bytes);
	int GetLeftDownBytes();
	int GetLeftUpBytes();
	void SetDownloadSpeedLimit(int speed);
	void SetUploadSpeedLimit(int speed);
	void SetConnectionLinkMax(unsigned int con);
	bool IsSelfPeerId(std::string& peerid);
	CTrackerCenter* GetTrackerCenter();
	int GetTimeToFinish();
	int GetDownloadSpeed();
	int GetUploadSpeed();
	void GetSumPieceSet( int *buf );
	unsigned int GetLeftPieceCount();
	int GetFinishedPieceCount();
	void NewDHTNode(unsigned int iip, unsigned short iport);
	int GetListenPort();
	void SetConnectingLinkMax(unsigned int con);
	unsigned int GetConnectingMax();
    void SetCacheSize( unsigned int count );
	CBTPiece& GetBitSet();
    void BroadcastHavePiece( int index );
    //void LogMsg( wchar_t* msg, int tid, _MSGTYPE type );
    void SetParent( CBTJob* parent );
    float CalculateFinishedPercent();
    void GetPrioritySet( int* buf );
    int GetFinishedPieceSet( int* buf );
    int GetAllPieceNumber();
    bool IsPriorityChanged();
    bool IsPieceSumChanged();
	bool IsFinishedPieceChanged();
    unsigned int GetUploadLinkMax();
    unsigned int GetLinkMax();
    void UnregisteSession( CBTSession* sess );
    unsigned int IsPieceInterest( CBTPiece& piece );
    bool IsPieceInterest( int index );
    bool IsPieceFinish( int index );
    bool GetPieceTask(  int index , bool& coorperate );
    unsigned int RegisteSession( CBTSession* sess );
    llong GetSumOfUpload();
    llong GetSumOfDownload();
    void SumUpUpload(unsigned int iip, int count );
    void SumUpDownload(unsigned int iip, int count );
    void PeerHaveNewPieceNotice(unsigned int iip, int index );
    void PieceChangeNotice( CBTPiece& bitset, bool newpeer );
    float GetAvailability();
    void InitFilePriority( std::string& prios );
    void AdjustFilePriority( const char* prios );
    BencodeLib::CTorrentFile* GetTorrentFile();

    void AbandonPieceTask( int index );
    bool Start();
    void Stop();
    bool IsFinished();
    int GetPieceLength( int index );
	bool GetPieceTask( CBTPiece& bitSet, int& piecetask, bool& coorperate );
    void WritePiece( int index, std::string& data );

    float GetFinishedPercent();
    llong GetSelectedCount();
    void SetDestPath( const wchar_t* path );
    bool IsFinishedPiece( int index );
    bool ReadData( std::string& data, int index, unsigned int offset, unsigned int len );
	bool AddNewPeer( unsigned int iip, unsigned short iport);
	bool IsPeerNeedMyPiece( CBTPiece& bitSet );

	void CheckOutOrphanData(int index, std::list<COrphan>& orphans);
	void CheckInOrphanData(int index, std::list<COrphan>& orphans);
	void CheckInOrphanData(int index, unsigned int iip, unsigned int offset, std::string& data);
	_JOB_STATUS GetJobStatus();
	void SetJobStatus(_JOB_STATUS status) {m_JobStatus=status;};
protected:
	int SmartWriteCache();
	std::string GenMyID( int seq );
	bool CheckEdgeIntegrality();
	bool IsFileInPreviewMode(int seq);
	int GetFilePriority(int seq);
	virtual void Entry();
    void CheckIsTaskFinished();
    //const wchar_t* GetIndexName();
    bool GetFilesReady();
    bool IsDownloadingPiece( int index );
	bool IsEdgePiece(int index);

};

#endif

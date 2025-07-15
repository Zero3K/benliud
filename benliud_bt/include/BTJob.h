/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// BTJob.h: interface for the CBTJob class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTJOB_H
#define _BTJOB_H

//2 sessions is much better than 1 session
//is 3 sessions better than 2 sessions?
//#define SESSIONNUM (3)
//3 sessions
//the first is normal
//the second is normal
//the third is always empty

//做第三个SESSION的测试，这个SESSION永远是没有数据的，专门针对
//种子连接并取得数据，这个SESSION不接受进入的连接，只对外发出连接
//发出的连接数不超过10个，使用完全随即的ID，


#include "../include/sessiondef.h"
#include "../include/BTStorage.h"
#include "../include/BTSession.h"
//#include "../../benliud/datatype_def.h"
#include <ThreadBase.h>
#include <Mutex.h>
#include "../../benliud/bittorrent_types.h"


//enum _JOB_STATUS
//{
//	_JOB_NONE,
//	_JOB_RUNNING,
//	_JOB_DOWNLOADING,
//	_JOB_UPLOADING,
//	_JOB_FAILED,
//	_JOB_QUIT,
//};

class CBTListener;

class CBTJob : public SockLib::CThreadBase
{

public:
	void SetSingleListener(CBTListener* listener);
	void SetConnectionRatio(float ratio);
	void SetConnectingRatio(float ratio);
	int GetMaxConnecting();
	int GetMaxConnection();
	void AdjustMaxConnecting(int conn);
	bool IsFinished();
	void AdjustMaxConnection(int conn);
	void AdjustEncryptMode(_BT_ENCMODE mode);
	void AdjustStopMode(_BT_STOPMODE mode);
	void AdjustCacheSize(int cache);
	void AdjustDownSpeed(int speed);
	void AdjustUpSpeed(int speed);
	void AdjustFilePriority( const char* prios );
	//信息提取接口
	int GetDownSpeed();
	int GetUpSpeed();
	float GetProgress();
	_JOB_STATUS GetStatus();
	float GetAvailability();


    void Stop();
    CBTJob( int taskid,
			int torsize,
			const char* torrent,
			int bitsetsize,
			const char* bitset,
            const wchar_t*	savepath,
            UINT	codepage,
            int	uplimit,
            int	downlimit,
			int conlimit,
			int connection,
			int cachesize,
			_BT_ENCMODE encmode,
			_BT_STOPMODE stopmode,
			int priosize,
            const char* prios);

    virtual ~CBTJob();
    bool Go();
	void AddPeers(int nbytes, const char* pdata);
protected:
    void Entry();
    bool Init();

private:
    CBTStorage m_Storage;
    CBTSession m_Session[ SESSIONNUM ];

	std::string m_Torrent; //save the torrent content
	std::string m_BitSet;	//initial bitset saved in db

    wchar_t m_SavePath[ MAX_PATH ];
    UINT m_CodePage;
    int m_UpLimit;
    int m_DownLimit;
    int m_nTaskId;
    bool m_bStop;

	SockLib::CMutex m_SetActiveMutex;
    //bool m_bActive; //an active job is selected by user, we need to notice pieces info
    int m_nAllPieceNumber;
	int m_nConLimit; //the connecting max
	int m_nConMax; //the connection max
	_BT_ENCMODE m_EncMode;
	_BT_STOPMODE m_StopMode;
	int m_nCacheSize;
	int m_nTrackerNum;
	bool m_ForceUpdate;

	int m_nSpecialStopForUnfinished;
	int m_nSpecialStopForFinished;

	std::string		m_FilePriority;

	CBTListener*	m_SingleListener;

};

#endif

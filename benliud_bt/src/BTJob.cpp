/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#include "stdafx.h"

// BTJob.cpp: implementation of the CBTJob class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/BTJob.h"
#include "../include/BTSession.h"
#include "../include/BTStorage.h"

#include <TorrentFile.h>
#include <Tools.h>
#include "../include/SpeedControl.h"
#include "../include/TrackerCenter.h"

#include <time.h>
extern void syslog(std::string info);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CBTJob::CBTJob(
    int taskid,
	int torsize,
	const char* torrent,
	int bitsetsize,
	const char* bitset,
    const wchar_t*	savepath,
    UINT	codepage,
    int	uplimit,
    int	downlimit,
	int conlimit,
	int maxconn,
	int cache,
	_BT_ENCMODE encmode,
	_BT_STOPMODE stopmode,
	int priosize,
    const char* prios )
{



	if(torrent!=NULL && torsize>0)
	{
		m_Torrent.append(torrent,torsize);
	}
	
	if(bitset!=NULL && bitsetsize>0)
	{
		m_BitSet.append(bitset, bitsetsize);
	}


    m_SavePath[ 0 ] = 0;
    wcscpy( m_SavePath, savepath );  //可能出问题


	m_CodePage= codepage;
	m_Storage.SetCodePage(codepage);

    m_UpLimit = uplimit;
    m_DownLimit = downlimit;

	m_nConMax=maxconn; //the connection max
	m_EncMode=encmode;
	m_nCacheSize=cache;
	m_nConLimit=conlimit;

    m_nTaskId = taskid;
    m_bStop = false;

 	m_nTrackerNum=0;
	m_ForceUpdate=false;
	m_StopMode=stopmode;

	if(prios!=NULL && priosize>0)
	{
		m_FilePriority.append(prios, priosize);
	}


	m_nSpecialStopForUnfinished=0;
	m_nSpecialStopForFinished=0;	

}

CBTJob::~CBTJob()
{
}

void CBTJob::AddPeers(int nbytes, const char* pdata)
{
	unsigned int iip;
	unsigned short iport;

	for(int i=0;i<nbytes/6;i++)
	{
		memcpy(&iip, pdata+i*6, 4);
		memcpy(&iport, pdata+i*6+4, 2);
		m_Storage.AddNewPeer(iip, iport);
	}
}

bool CBTJob::Go()
{
    return Run(false);  //非分离线程
}

void CBTJob::Entry()
{

	if(!Init())
	{
		m_Storage.SetJobStatus(_JOB_FAILED);
		return;
	}

    while ( !m_bStop )
    {//circle every one second

#if defined( WIN32)||defined(WINCE)
		Sleep( 100 );
#else
		Tools::Sleep(100);
#endif

	}


    for ( int sess = 0;sess < SESSIONNUM;sess++ )
    {
        m_Session[sess].Stop();
    }

    for ( int sess = 0;sess < SESSIONNUM;sess++ )
    {
        m_Session[ sess ].Wait();
    }

    m_Storage.Stop();
	
	m_Storage.SetJobStatus(_JOB_QUIT);

}

//mark the stop and leave , wait the stop notice
void CBTJob::Stop()
{

    m_bStop = true; 

}

bool CBTJob::Init()
{

    m_Storage.SetParent( this );

	m_Storage.SetSingleListener(m_SingleListener);

    m_Storage.SetDestPath( m_SavePath );
	m_Storage.SetInitBitSet(m_BitSet);
	m_Storage.SetCodePage(m_CodePage);

	if(!m_Storage.ReadTorrentContent(m_Torrent, m_CodePage))
    {
	
        return false;
    }

	m_Storage.SetConnectingLinkMax(m_nConLimit);
	m_Storage.SetConnectionLinkMax(m_nConMax);
	m_Storage.SetCacheSize(m_nCacheSize);

	m_nTrackerNum=m_Storage.GetTorrentFile()->GetAnnounceNumber()+1; //+1 for DHT

    m_Storage.InitFilePriority( m_FilePriority );
	m_Storage.SetUploadSpeedLimit(m_UpLimit);
	m_Storage.SetDownloadSpeedLimit(m_DownLimit);

    if ( !m_Storage.Start() )
    {
        return false;
    }

	int sess;

    for ( sess = 0;sess < SESSIONNUM;sess++ )
	{
		m_Session[ sess ].SetStorage( &m_Storage );
		m_Session[ sess ].SetEncryptMode(m_EncMode);

	}


    for ( sess = 0;sess < SESSIONNUM;sess++ )
    {
       if ( !m_Session[ sess ].Start() )
        {
            return false;
        }

    }

    m_nAllPieceNumber = m_Storage.GetAllPieceNumber();

    return true;
}

void CBTJob::AdjustFilePriority( const char* prios )
{
	m_FilePriority=prios;

//	LogMsg(L"adjust priority in CJob",0,MSG_INFO);
	m_Storage.AdjustFilePriority(prios);

}


void CBTJob::AdjustUpSpeed(int speed)
{
	m_UpLimit=speed;

	m_Storage.SetUploadSpeedLimit(speed);
}

void CBTJob::AdjustDownSpeed(int speed)
{
	m_DownLimit=speed;

	m_Storage.SetDownloadSpeedLimit(speed);
}

void CBTJob::AdjustCacheSize(int cache)
{
	m_nCacheSize=cache;
	m_Storage.SetCacheSize(m_nCacheSize);
}

void CBTJob::AdjustStopMode(_BT_STOPMODE mode)
{
	m_StopMode=mode;
}

void CBTJob::AdjustEncryptMode(_BT_ENCMODE mode)
{
	m_EncMode=mode;
    for ( int i = 0;i < SESSIONNUM;i++ )
	{
		m_Session[ i ].SetEncryptMode(m_EncMode);
	}
}

void CBTJob::AdjustMaxConnection(int conn)
{
	m_nConMax=MAX(conn,1);
	m_Storage.SetConnectionLinkMax(m_nConMax);
}

bool CBTJob::IsFinished()
{
	return m_Storage.IsFinished();
}

void CBTJob::AdjustMaxConnecting(int conn)
{
	m_nConLimit=MAX(conn,1);
	m_Storage.SetConnectingLinkMax(m_nConLimit);
}

int CBTJob::GetMaxConnection()
{
	return m_nConMax;
}

int CBTJob::GetMaxConnecting()
{
	return m_nConLimit;
}

//when more than one task running, we need adjust the ratio
void CBTJob::SetConnectingRatio(float ratio)
{
	m_Storage.SetConnectingRatio(ratio);
}

void CBTJob::SetConnectionRatio(float ratio)
{
	m_Storage.SetConnectionRatio(ratio);
}

void CBTJob::SetSingleListener(CBTListener *listener)
{
	m_SingleListener=listener;
}

//信息提取接口
int CBTJob::GetDownSpeed()
{
	return m_Storage.GetDownloadSpeed();
}

int CBTJob::GetUpSpeed()
{
	return m_Storage.GetUploadSpeed();
}

float CBTJob::GetProgress()
{
	return m_Storage.GetFinishedPercent();
}

_JOB_STATUS CBTJob::GetStatus()
{
	return m_Storage.GetJobStatus();
}

float CBTJob::GetAvailability()
{
	return m_Storage.GetAvailability();
}
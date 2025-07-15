/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#if defined(WINCE)
#include <windows.h>
#elif defined(WIN32)
#pragma warning (disable: 4786)

#include <io.h> //low level file opt.
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <netdb.h> //gethostbyname
#include <sys/socket.h>
#include <sys/unistd.h>
#include <arpa/inet.h>  //inet_ntoa
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


#include <memory>
#include <time.h>

#include "../include/BTStorage.h"
#include "../include/BTSession.h"

#include <Tools.h>
#include <AutoLock.h>
#include <SHA1.h>
#include <TorrentFile.h>

#include "../include/BTJob.h"
#include "../include/filewrap.h"
#include "../include/BTJob.h"
#include "../include/TrackerCenter.h"
#include "../include/BTListener.h" //single port listener

extern void syslog(std::string);

#define MIN(a,b) ((a)>(b)?(b):(a))

//the priority > 5 will use more than one ant to get data
//the ants should coorperate
#define MULTIANT_PRIO	(5)  //it's the threshold of multi-ant piece!

CBTStorage::CBTStorage()
{
    m_pTorrentFile = NULL;
    m_pParent = NULL;
    m_nReadHit = 0;
    m_nReadCacheHit = 0;
    m_nBanedSize = 0;
    m_nSumOfDownload = 0;
    m_nSumOfUpload = 0;
    m_nCacheSize = 10 * 1024 * 1024; //default cache
    m_fPieceAvailability = 0.0f;
	m_fFinishedPercent = 0.0f;
	m_nConnectingLinkMax = 10; //default to 10
	m_nConnectionLinkMax = 200;
	m_SessionIdCounter=0;
	m_nListenSocket=-1;
	m_nListenPort=0;
	
    m_bPieceSumChanged = false;
    
    m_bPriorityChanged = false;
    m_bIsTaskFinished = false;
	m_JobStatus = _JOB_NONE;

	m_UpByteCount=0;
	m_UploadSpeedLimit=0x0FFFFFFF;
	m_DownByteCount=0;
	m_DownloadSpeedLimit=0x0FFFFFFF;
	
    m_sIndexPath[ 0 ] = wchar_t( 0 );
	
	m_nLastTickForCalSpeed=0; //last tick count we cal ave speed
	m_nTimeToFinish=-1;  //cal by averagespeed and left piece
	m_nAverageSpeed=0;  //average download speed in bytes/s
	m_nAverageUpSpeed=0; //average upload speed in bytes/s

	m_nWasteByte=0;
	m_nDHTPort=0;
	
	m_fConnectionRatio=1.0f;
	m_fConnectingRatio=1.0f;

	m_pSingleListener=NULL;
	
    srand( GetTickCount() );
	

	//ŽÓ²âÊÔ¿ŽÀŽ£¬¶àžöPEERID»¹ÊÇÐèÒªµÄ£¬
	//ÓÐÐ©ÖÖ×ÓŽóžÅ·Ž¶ÔÒ»žöµØÖ·ÉÏµÄ¶àÁ¬œÓ£¬µŒÖÂÏÂÔØ¹ý³ÌÖÐ»áÊ§È¥¶ÔÖÖ×ÓµÄÁ¬œÓ£¬ËÙ¶ÈÏÂœµ

	//prebuild the session id
	//the first one is normal one
	//session id ÓŠžÃ¿ÉÒÔ±»×ÔŒºÊ¶±ðÕâÑù¿ÉÒÔÔÚœ«ÀŽÓÐÓÅÊÆ

	//×Ü¹²20žö×ÖœÚÕâÑù×ö£º
	//[0-5]=°æ±ŸŒ°±êÊ¶ŒÓÃÜ, ŒÓÃÜÓÃ17£¬18Áœžö×ÖœÚÀŽÒì»ò
	//[6-15]Ëæ»ú£¬[16] ÊÇÇ°[6-15]×ÖœÚµÄÒì»òºÍ 
	//[17]ÊÇËæ»ú£¬µ«µÚ2Î»ÓÀÔ¶ÊÇ0£¬µÚ6Î»ÓÀÔ¶ÊÇ1
	//[18]ÊÇËæ»ú£¬µ«µÚ3Î»ÓÀÔ¶ÊÇ0£¬µÚ7Î»ÓÀÔ¶ÊÇ1
	//[19]ÊÇÇ°Ãæ19×ÖœÚµÄÒì»òºÍ
	//Âú×ãÒÔÉÏÌõŒþµÄÊÇ¿ÉÄÜµÄÍ¬°é£¬Æäºó·¢ËÍÅÐ¶ÏÃüÁîÀŽÈ·ÈÏÊÇ·ñÊÇmonma¿Í»§¶Ë
/*
	for(int id=0;id<SESSIONNUM;id++)
	{
		std::string idstr=GenMyID();
		m_MyIdList.push_back(idstr);
	}
*/
}

CBTStorage::~CBTStorage()
{
    if ( m_pTorrentFile != NULL )
    {
        delete m_pTorrentFile;
    }
	

}

bool CBTStorage::OpenFiles()
{
    BuildFileListStructure();
    return GetFilesReady();
}


//保存改成由上层来做
//void CBTStorage::SaveIndex()
//{

	//every bitset change we report to UI, UI then saved to database
	//std::string bitset=m_FinishedBitSet.GetStream();

	//m_pTaskPiece(m_nTaskId, _PIECE_SAVE, 
	//	bitset.size(), 
	//	(int*)(bitset.data()), 0.0);

//}


void CBTStorage::CheckOldFiles()
{

    unsigned int count = m_pTorrentFile->GetPieceCount();
#if defined( WIN32)||defined(WINCE)
	SetPriority(THREAD_PRIORITY_IDLE);
#endif
	std::string piecedata;

    for ( unsigned int i = 0; i < count; i++ )
    {

		//no need to buffer these data
        if(!ReadPieceWithoutBuffer( piecedata, i )) continue;
		
		std::string hash=Tools::SHA1String(piecedata);
		
		if(m_bStop) {
#if defined( WIN32)||defined(WINCE)
			SetPriority(THREAD_PRIORITY_NORMAL);
#endif
			return;
		}

        if ( hash == m_pTorrentFile->GetPieceHash( i ) )
        {
            m_FinishedBitSet.Set( i, true );
        }
        else
        {
            m_FinishedBitSet.Set( i, false );
        }

		m_fFinishedPercent = CalculateFinishedPercent();


    }
#if defined( WIN32)||defined(WINCE)
	SetPriority(THREAD_PRIORITY_NORMAL);
#endif

}

bool CBTStorage::IsNeedCheckFiles()
{
	
	unsigned int total = m_pTorrentFile->GetPieceCount();

	unsigned int bytes= total/8; //total/8 + (total%8)?1:0 is wrong!;

	bytes+=((total%8)?1:0); 
	
	if(m_InitBitSet.size()==bytes)
	{
		m_FinishedBitSet.Init(m_InitBitSet, total);
		return false;
	}
	else
	{
		//int check=m_InitBitSet.size();
		return true;
	}
	

}


bool CBTStorage::Start()
{
	m_bStop=false;
    m_FinishedBitSet.Init( m_pTorrentFile->GetPieceCount() );
    m_bitPieceSum.Init( m_pTorrentFile->GetPieceCount() );
	
	m_nLastTickForCalSpeed=0; //last tick count we cal ave speed
	m_nTimeToFinish=-1;  //cal by averagespeed and left piece
	m_nAverageSpeed=0;  //average download speed in bytes/s
	
    m_bOldFileExists = false;
	
    m_nBanedSize = 0;
	m_SessionIdCounter=0;
	
    if ( !OpenFiles() )
    {
		m_JobStatus=_JOB_FAILED;
        return false;
    }
	

    GenPriorityBitSet(); //
	

#ifdef _SINGLEID
	std::string ids=GenMyID(0);
	memcpy(m_MyId,ids.data(),20);
#else
	for(int i=0;i<SESSIONNUM;i++)
	{
		std::string ids=GenMyID(i);
		memcpy(m_MyId+20*i,ids.data(),20);
	}
#endif
	
	//so we know the ip:port we are start on, start the TrackerCenter
	m_pTrackerCenter=new CTrackerCenter(this);

	return Run(false);
}

//call by the thread ,moved out from start()
void CBTStorage::JobInit()
{
	
    if ( m_bOldFileExists && IsNeedCheckFiles())  //have old file with same length
    {

		m_JobStatus=_JOB_CHECKINGFILE;
        CheckOldFiles();
		m_JobStatus=_JOB_RUNNING;

		//TOFIX, save that bitset
		//SaveIndex();
    }
	else
	{//no need check integrality if check old files

		m_JobStatus=_JOB_CHECKINGFILE;
		CheckEdgeIntegrality();
		m_JobStatus=_JOB_RUNNING;
	}

    //set to true to update the initial state to GUI
	
    m_bPieceSumChanged = true;
	
    m_bPriorityChanged = true;
	
    m_fFinishedPercent = CalculateFinishedPercent();

    CheckIsTaskFinished();

	if(IsFinished())
	{
		m_PeerCenter.DownloadFinish(true);
		m_JobStatus=_JOB_UPLOADING;
	}
	else
	{
		m_JobStatus=_JOB_DOWNLOADING;
	}
	

	InitFileProgress();


	TSessionList::const_iterator it;
	m_SessionListMutex.Lock();
	for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
	{
		( *it ) ->DownloadFinish(m_bIsTaskFinished);
	}
	m_SessionListMutex.Unlock();

	m_nListenPort=m_pSingleListener->GetListenPort(); //ÕâÀïµœÉÏ²ãÈ¡ŒàÌý¿Ú

	m_pTrackerCenter->Start();
	
}

void CBTStorage::Stop()
{

	m_bStop=true;
	

	Wait();//wait for our listen thread quit

	
	
	m_pTrackerCenter->Stop();
	delete m_pTrackerCenter;
	m_pTrackerCenter=NULL;
	
	
    TFileHandleList::iterator it;
	
    for ( it = m_FileHandleList.begin(); it != m_FileHandleList.end(); ++it )
    {
#if defined(WINCE)||defined(WIN32)
		if ( it->Priority == 0 || it->handle == INVALID_HANDLE_VALUE )
#else
		if ( it->Priority == 0 || it->handle == -1 )
#endif
        {
            continue;
        }
		
#if defined(WINCE)||defined(WIN32)
		::CloseHandle(it->handle);
#else
        close( it->handle );
#endif
		
    }
	

}


//use this func to adjust the file priority at any time
//so not just replace the list in class
//only call it in the middle of progress
//call InitFilePriority before run
//Ò»¶ÔÒ»Æ¥ÅäÊµÎÄŒþ£¬UI²ãÃæ¿Ž²»µœÐéÎÄŒþ£¬ËùÒÔÒª±Ü¿ªÐéÎÄŒþÀŽÉèÖÃÓÅÏÈŒ°
void CBTStorage::AdjustFilePriority( const char* prios )
{
	//ÐÂµÄŽŠÀí...
	std::string newprios=prios;

	//compare to the old one check if something changed
	//assert(m_FilePriority.size()==newprios.size());
	
	
	if(newprios!=m_FilePriority)
	{//changed
		m_FilePriority=newprios;
        //rebuild something
        if(RebuildFileListStructure())
		{
			CheckEdgeIntegrality();
		}
		else
		{
			//printf("no new file opened\n");
		}
		

        GenPriorityBitSet();
	
        m_fFinishedPercent = CalculateFinishedPercent();


        m_bPriorityChanged = true;

		bool oldstatus=IsFinished();

        CheckIsTaskFinished();
		
		if(IsFinished())
		{
			m_JobStatus=_JOB_UPLOADING;
		}
		else
		{
			m_JobStatus=_JOB_DOWNLOADING;
		}

		if(oldstatus!=IsFinished())
		{
			//Œì²éÕâÀïÊÇ·ñ»áÓÐÎÊÌâ£¡
			TSessionList::const_iterator it;
			m_SessionListMutex.Lock();
			for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
			{
				
				( *it ) ->DownloadFinish(IsFinished()); //change to the upload mode
				
			}
			m_SessionListMutex.Unlock();
		}
		
		
	}


}

bool CBTStorage::IsFinished()
{
    return m_bIsTaskFinished;
}

CBTPiece& CBTStorage::GetBitSet()
{
    return ( m_FinishedBitSet );
}

//the last piece maybe not a full piece length
int CBTStorage::GetPieceLength( int index )
{
	assert( index >=0);
    assert( index < m_pTorrentFile->GetPieceCount() );

	if(index != m_pTorrentFile->GetPieceCount() -1)
	{//²»ÊÇ×îºóÒ»Æ¬µÄÖÐŒäÆ¬¶ŒÊÇ¶š³€
		return m_pTorrentFile->GetPieceLength();
	}
	else
	{//×Ü³€¶ÔÆ¬³€ÇóÓàÊýÓŠžÃ¶Ô
		int left= (int)( (m_pTorrentFile->GetTotalSize(true /*with virtual file*/)) % (m_pTorrentFile->GetPieceLength()));
		return (left)?left:(m_pTorrentFile->GetPieceLength());
	}

}

bool CBTStorage::GetPieceTask( CBTPiece& bitSet, int& piecetask, bool& coorperate )
{
    //make a copy of m_PriorityBitSet to use
    CBTPieceSum pc = m_PriorityBitSet;
	
    //mark off peer's bitset
    for ( unsigned int j = 0; j < pc.GetSize(); j++ )
    {
        if ( !bitSet.IsSet( j ) )
            pc.SetValue( j, 0 ); //mark all unset bit
    }
	
    //mark off finished pieces
    for ( unsigned int n = 0; n < pc.GetSize(); n++ )
    {
        if ( m_FinishedBitSet.IsSet( n ) )
            pc.SetValue( n, 0 ); //mark all unset bit
    }

	CBTPieceSum pc2=pc; //²»ÅÅ³ýÕýÔÚÏÂÔÚµÄÈÎÎñµÄž±±Ÿ

	//ÅÅ³ýËùÓÐÕýÔÚÏÂÔÚµÄÈÎÎñ,³ý·ÇÕâžöÈÎÎñÊÇžßÓÅÏÈ¿ÉÖØµþµÄ
	{
		TDownloadingMap::iterator it;
		m_DownloadingMapMutex.Lock();
		for ( it = m_DownloadingMap.begin(); it != m_DownloadingMap.end(); it++ )
		{
			int prio=m_PriorityBitSet.GetValue(it->first);
			if( prio > 10 && it->second <=2)
			{
			}
			else if( prio > 5 && it->second <=1)
			{
			}
			else
			{
				pc.SetValue(it->first,0);
			}
		}
		m_DownloadingMapMutex.Unlock();
	}

	//Ê£ÓàÖÐËÑË÷×îžßÓÅÏÈŒ¶µÄÈÎÎñ
	int maxprio=0;
	for( unsigned int m=0; m < pc.GetSize(); m++)
	{
		if(pc.GetValue(m) > maxprio) maxprio=pc.GetValue(m);
	}

	if(maxprio > 0)
	{//ÓÐ¿ÉÏÂµÄÆ¬£¬ËÑË÷¹Â¶ùÆ¬ÄÚÊÇ·ñÓÐÓÅÏÈŒ¶ÏàÍ¬»òžüžßµÄÈÎÎñ£¬Èç¹ûÓÐÔòÓÅÏÈÏÂÔØ¹Â¶ùÆ¬


		//ÔÚ¹Â¶ùÄÚËÑË÷×îŽóÓÅÏÈŒ¶±ðµÄÈÎÎñ
		int o_maxidx=-1;
		int o_maxprio=0;

		{
			SockLib::CAutoLock al(m_OrphanMapMutex);

			TOrphanMap::const_iterator it;
			for(it=m_OrphanMap.begin();it!=m_OrphanMap.end();it++)
			{
				if( pc.GetValue(it->first) > o_maxprio )
				{
					o_maxidx=it->first;
					o_maxprio=pc.GetValue(it->first);
				}
			}
		}

		if(o_maxprio >= maxprio)
		{//¹Â¶ùÖÐŽæÔÚžüÓÅÏÈµÄÈÎÎñ£¬Ö±œÓ·µ»ØÕâžöÈÎÎñ
			piecetask=o_maxidx; 
		}
		else
		{//¹Â¶ùÈÎÎñÃ»ÓÐžüžßÓÅÏÈŒ¶µÄ£¬ÔÚÊ£ÓàµÄÆ¬ÖÐËæŒŽÈ¡Ò»žöÓÅÏÈ×îžßµÄÆ¬·µ»Ø
			//œ«×îžßÓÅÏÈŒ°µÄÈÎÎñ×é³ÉÒ»žö¶ÓÁÐ
			std::vector<int> MaxPriorityList;
		
			for ( unsigned int m = 0; m < pc.GetSize(); m++ )
			{
				if ( pc.GetValue( m ) == maxprio )
					MaxPriorityList.push_back( m );
			}

			int lsize = MaxPriorityList.size();

			assert(lsize > 0);

			int randp = 0;

			//¶àŽó±ÈÀýµÄÑ¡ÔñÈ«ŸÖœÏÉÙµÄÆ¬£¬ÓŠ²Î¿ŒÈ«ŸÖ¿œ±ŽÊým_fPieceAvailability¶ø¶š
			//È«ŸÖ¿œ±ŽÊýÐ¡£¬ÔòÓŠžÃ¶àÑ¡È«ŸÖ×îÉÙµÄÆ¬ÀŽÏÂ£¬¶Ô³õÊŒœ×¶ÎµÄÏÂÔØ±ÈœÏ¹ØŒü
			int pickleast = 80;

			if(m_fPieceAvailability < 2.0)
			{
				pickleast = 90; //90%Çé¿öÏÂÑ¡Ôñ×îÉÙÆ¬ÏÂ
			}
			else if(m_fPieceAvailability < 5.0)
			{
				pickleast = 80; //80Çé¿öÏÂÑ¡×îÉÙÏÂ
			}
			else if(m_fPieceAvailability < 10.0)
			{
				pickleast = 70; //70%Çé¿öÏÂÑ¡×îÉÙÏÂ
			}
			else if(m_fPieceAvailability < 20.0)
			{
				pickleast = 55; //50%Çé¿öÏÂÑ¡×îÉÙÏÂ
			}		
			else if(m_fPieceAvailability < 30.0)
			{
				pickleast = 40; //40Çé¿öÏÂÑ¡×îÉÙÏÂ
			}
			else if(m_fPieceAvailability < 60.0)
			{
				pickleast = 30; //30%Çé¿öÏÂÑ¡×îÉÙÏÂ
			}
			else
			{
				pickleast = 15; //20%Çé¿öÏÂÑ¡×îÉÙÏÂ
			}
			
			if( (rand() % 100) > pickleast ) 
			{
				//Ëæ»úÑ¡ÔñÒ»žöÆ¬
				randp = rand() % lsize;  //rand maybe small than lsize,but no problem
			}
			else
			{
				//ÔÚ¿ÉÑ¡·¶Î§ÄÚÑ¡ÔñÒ»žö×îÉÙÊýŸÝÆ¬ÏÂÔØ
				//Ñ¡ÔñÒ»žöÈ«ŸÖ×îÉÙµÄÊýŸÝÆ¬
				int gmin=0x0FFFFFFF; //È«ŸÖ×îÉÙ
				std::vector<unsigned int> leastlist;

				for(/*unsigned*/ int p=0; p < lsize; p++)
				{
					//ÕâžöÆ¬¶ÔÓŠµÄÈ«ŸÖž±±ŸÊý , œ«×îÉÙž±±ŸÊýµÄÆ¬ÐòºÅ·Åœø±íÖÐËæŒŽÌôÑ¡Ò»žö
					int pnum = m_bitPieceSum.GetValue(MaxPriorityList[p]);
					if( pnum < gmin )
					{
						gmin=pnum;
						leastlist.clear();
						leastlist.push_back(p);
					}
					else if( pnum == gmin )
					{
						leastlist.push_back(p);
					}
				}

				randp= leastlist[rand()%leastlist.size()];

			}

			piecetask=MaxPriorityList[randp];

		}

		//ÈÎÎñÒÑŸ­È·¶š£¬ŒÇÂŒÕýÔÚÔËÐÐÖÐµÄÈÎÎñ
		{
			SockLib::CAutoLock al(m_DownloadingMapMutex);
			TDownloadingMap::iterator it2;
			it2=m_DownloadingMap.find(piecetask);

			if(it2!=m_DownloadingMap.end()) {
				it2->second++;
				coorperate=true;
			}
			else
			{
				m_DownloadingMap[piecetask]=1;
				if(m_PriorityBitSet.GetValue(piecetask)> 5)
				{
					coorperate=true;
				}
				else
				{
					coorperate=false;
				}
			}
		}

		assert(piecetask>=0);
		assert(piecetask<m_pTorrentFile->GetPieceCount());

		if(GetLeftPieceCount() < 2*m_DownloadingMap.size())
		{
			coorperate=true;
		}

		return true;
	}
	else
	{//Ã»ÓÐ¿ÉÑ¡µÄÈÎÎñ¿ÉÏÂ£¬Ö»ÓÐÔÚÕýÔÚÏÂÔÚµÄÆ¬ÀïÑ¡Ò»žöÀŽ×ö£¬Èç¹û»¹Ñ¡²»µœ£¬Ôò·ÅÆú
		//ŽËÊ±¿ÉÄÜÈÎÎñÒÑŸ­¿ìœáÊøÁË¡£Èç¹ûÊÇÈÎÎñ¿ìœáÊøµÄ×ŽÌ¬£¬ÄÇÃŽËùÓÐµÄÎŽÍê³ÉÆ¬¶ŒÒÑŸ­
		//ÔÚÇëÇóÁÐ±íÀï£¬ŽËÊ±Ò»¶š°ïÖúÏÂÔÚÆäËûÆ¬£¬Èç¹û²»ÊÇ¿ìœáÊøÁË£¬Ôò°ŽÓÅÏÈŒ¶£¬Ñ¡Ôñ°ïÖú
		//žßÓÅÏÈŒ¶ÈÎÎñÖØµþÇëÇó£¬Èç¹û²»ÊÇÔÚœáÊøÄ£ÊœÏÂ£¬ÆÕÍšÓÅÏÈŒ¶µÄÈÎÎñ²»ÒªÈ¥ÖØµþÇëÇó
		//ÕâÑùÈÝÒ×Ôì³É²»±ØÒªµÄŽø¿íÀË·Ñ¿š×¡ÆäËûÈÎÎñÏÂÔØ

		//LogMsg(L"find task in downloading",0,MSG_INFO);

		SockLib::CAutoLock al(m_DownloadingMapMutex);

		if(m_DownloadingMap.empty()) {
			//LogMsg(L"no task downloading",0,MSG_INFO);
			return false;
		}

		//unsigned int left=GetLeftPieceCount();

		//if( left >= m_DownloadingMap.size() + 10 )

		if( GetLeftPieceCount() > m_DownloadingMap.size() * 2 )
		{//not in ending mode
			//Ñ¡ÔñžßÓÅÏÈÈÎÎñÖØµþÏÂÔØ£¬°ŽË³ÐòÕÒ£¬ÄÄžö¿ÉÒÔÖØµþÏÂÔØŸÍÏÂÄÄžö
			//LogMsg(L"not in ending mode",0,MSG_INFO);

			TDownloadingMap::iterator it;

			for(it=m_DownloadingMap.begin();it!=m_DownloadingMap.end();it++)
			{
				int prio=pc2.GetValue(it->first);

				if(prio > 10 && it->second <=2)
				{
					//LogMsg(L"find a task with prio >10",0,MSG_INFO);

					piecetask=it->first;

					coorperate=true;
					it->second++;
					return true;
				}
				else if(prio > 5 && it->second <=1)
				{
					//LogMsg(L"find a task with prio >5",0,MSG_INFO);
					piecetask=it->first;

					coorperate=true;
					it->second++;
					return true;
				}

			}

			return false;

		}
		else
		{//ending mode


			TDownloadingMap::iterator it;
			int overlapidx=-1;
			int overlap=20; //¿Éµ÷œÚµœ10

			for(it=m_DownloadingMap.begin();it!=m_DownloadingMap.end();it++)
			{
				if(it->second==0) {

					return false;
				}

				if( pc2.GetValue(it->first) > 0 && it->second < overlap)
				{
					overlap=it->second;
					overlapidx=it->first;
				}
			}

			if(overlapidx==-1) {
				return false; //Ã»ÕÒµœºÏÊÊÈÎÎñ
			}else {
	
				m_DownloadingMap[overlapidx]++;

//				wchar_t msg[128];
//				swprintf(msg,L"return a overlap task idx=%d,overlap=%d", overlapidx,m_DownloadingMap[overlapidx]);
//				LogMsg(msg,0,MSG_INFO);

				piecetask=overlapidx;
				coorperate=true;
				return true;
			}
		}

	}

}


void CBTStorage::AbandonPieceTask( int index )
{

	if(index<0 || index >= m_pTorrentFile->GetPieceCount())
	{
		return;
	}

	SockLib::CAutoLock al(m_DownloadingMapMutex);

	TDownloadingMap::iterator it;

	it=m_DownloadingMap.find(index);

	if(it!=m_DownloadingMap.end())
	{
		it->second--;
		if(it->second <= 0)
		{
			m_DownloadingMap.erase(index);
		}
	}

}

//×îºóÒ»Æ¬¿ÉÄÜ²»ÊÇÒ»žöÍêÕûµÄÆ¬³€
void CBTStorage::WritePiece( int index, std::string& data )
{

	CleanOrphan(index);
	
    m_WriteCacheMutex.Lock();

    if ( m_FinishedBitSet.IsSet( index ) )  //read ,can do without lock
    {
		m_nWasteByte+=data.size();
		
		m_WriteCacheMutex.Unlock();

        return ;
    }
	
    if ( m_PriorityBitSet.GetValue( index ) == 0 )
    {
		m_nWasteByte+=data.size();

		m_WriteCacheMutex.Unlock();

        return ;
    }
	
		
	m_FinishedBitSet.Set( index, true );

	//ŒÇÂŒÎÄŒþœø¶È±ä»¯£¬º¬ÎÄŒþœø¶ÈÍšÖª
	RecordNewPieceInFile(index);

	m_WriteCache[index]=data; //Ã»±ØÒªŒì²âÊÇ·ñÖØžŽ£¬ÒòÎªfinishbitsetÒÑŸ­ÓÐ±êŒÇÁË

    m_WriteCacheMutex.Unlock();

	m_EdgeCacheMutex.Lock();
	if(IsEdgePiece(index))
	{
		m_EdgeCache[index]=data;
	}
	m_EdgeCacheMutex.Unlock();
	
    m_fFinishedPercent = CalculateFinishedPercent();

    CheckIsTaskFinished();  //m_FinishedBitSet changed so check is finished
	
    BroadcastHavePiece( index );  //notice all session we have this piece
	
	
    if ( IsFinished() )
    {
		m_JobStatus=_JOB_UPLOADING; //下载完成了

		m_PeerCenter.DownloadFinish(true);

//      LogMsg( L"========Download finished========", 0, MSG_INFO );
		
		//send event to all session
		bool ball=m_FinishedBitSet.IsAllSet(); //if we finish all the data, send complete event
		
		if(ball) 
		{
			m_pTrackerCenter->SetEvent(TE_COMPLETE);
		}
		
		TSessionList::const_iterator it;
		m_SessionListMutex.Lock();
		for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
		{
			
			( *it ) ->DownloadFinish(true); //change to the upload mode
			
		}
		m_SessionListMutex.Unlock();
		
    }

}

//when write cache moved to thread ,no need this function
//it just use when stop


int CBTStorage::WritePieceToDisk( int index, std::string& data )
{

	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());

	//ÕâÀïŒì²éÒ»ÏÂ£¬ÊýŸÝµÄ³€¶ÈÊÇ·ñ·ûºÏ¡£

	int dlen=GetPieceLength(index);

	if(dlen!=data.size()) 
	{
		return -1;
	}

	llong datahead= llong(index) * llong(m_pTorrentFile->GetPieceLength());
	llong datarear= llong(data.size()) + datahead;

	llong writepos=datahead;

	SockLib::CAutoLock al(m_FileSystemLock);


	while(writepos < datarear)
	{
		TStorageFileInfo sFileInfo;

		if(!GetFileInfoByOffset( sFileInfo, writepos )) //Ò»¶š²»ÄÜ·µ»ØÒ»žö³€¶ÈÎª£°µÄÎÄŒþ
		{
			return -2;//ÑÏÖØŽíÎó
		}

		assert(sFileInfo.fileInfo.size>0);

#if defined(WINCE)||defined(WIN32)
		if ( sFileInfo.Priority == 0 || sFileInfo.handle == INVALID_HANDLE_VALUE || sFileInfo.fileInfo.vfile  )
#else
        if ( sFileInfo.Priority == 0 || sFileInfo.handle == -1 || sFileInfo.fileInfo.vfile  )
#endif
        {
            writepos=sFileInfo.fileInfo.offset + sFileInfo.fileInfo.size; 
            continue;
        }

		assert(writepos >= sFileInfo.fileInfo.offset);
		assert(writepos < sFileInfo.fileInfo.offset + sFileInfo.fileInfo.size );

		llong fileoffset= writepos - sFileInfo.fileInfo.offset; //ÎÄŒþÄÚµÄÆ«Î»£¬ŽÓÕâÀï¿ªÐŽ
		unsigned int dataoffset= (unsigned int)(writepos-datahead); //ÊýŸÝÆ«ÒÆÎ»ÖÃ


		//unsigned int nbytes=MIN(sFileInfo.fileInfo.size - fileoffset, datarear - writepos); //ÎÄŒþ³€¶ÈºÍÊ£ÓàÊýŸÝ³€¶ÈÈ¡Ð¡
		unsigned int nbytes=0;
		if(sFileInfo.fileInfo.size - fileoffset > datarear - writepos)
		{
			nbytes= (unsigned int)(datarear - writepos);
		}
		else
		{
			nbytes= (unsigned int)(sFileInfo.fileInfo.size - fileoffset);
		}

#if defined(WINCE)||defined(WIN32)
		LARGE_INTEGER li;
		li.QuadPart=fileoffset;
		DWORD dwRet=::SetFilePointer(sFileInfo.handle, li.LowPart, &li.HighPart, FILE_BEGIN); 
		if(dwRet==0xFFFFFFFF && ::GetLastError()!=NO_ERROR)
		{
			//OutputDebugString(L"Fail to Seek file pos");
			return -4;
		}

#else	//linux
        llong check1=lseek64( sFileInfo.handle, fileoffset, SEEK_SET ); //ÒÆ¶¯µœÎÄŒþÄÚÆ«ÒÆ
		//The library routine llseek() is available in libc5 and glibc and works without special defines. Its prototype was given in <unistd.h>
		//llong check1=llseek( sFileInfo.handle, fileoffset, SEEK_SET ); //ÒÆ¶¯µœÎÄŒþÄÚÆ«ÒÆ
		if(check1!=fileoffset)
		{
			return -4;
		}
#endif


#if defined(WINCE)||defined(WIN32)
		DWORD dwWrote;
		BOOL bRet=::WriteFile(sFileInfo.handle, data.data()+ dataoffset, nbytes, &dwWrote, NULL);
		if(!bRet) {
			//OutputDebugString(L"Write file data failed");
			return -5;
		}

		writepos+= llong(nbytes);

#else
        int check2 = write( sFileInfo.handle, data.data() + dataoffset, nbytes );
		if(check2!=nbytes) 
		{
			return -5;
		}
		writepos+=llong(check2);
#endif

		
	}

	return 0;
}

bool CBTStorage::ReadData( std::string& data, int index, unsigned int offset, unsigned int len )
{

	m_nReadHit++;


	if(ReadDataFromCache(data,index,offset,len))
	{
		m_nReadCacheHit++;
		return true;
	}
	
	unsigned int piecelen=GetPieceLength( index ); 
	unsigned int readlen2=piecelen-offset;	
	unsigned int readlen=MIN(2*BUFFER_BLOCK_SIZE,readlen2); 

	std::string diskdata;
	if(!ReadDataFromDisk(diskdata,index,offset,readlen))
		return false;
	

	data=diskdata.substr(0,len);

	for(int i=0;!diskdata.empty();i++)
	{
		TBufferBlock newblock;
		newblock.pIndex=index;
		newblock.offset=offset+i*BUFFER_BLOCK_SIZE;

		if(diskdata.size() >=BUFFER_BLOCK_SIZE)
		{
			newblock.data=diskdata.substr(0,BUFFER_BLOCK_SIZE);
			diskdata.erase(0,BUFFER_BLOCK_SIZE);

			m_ReadCacheMutex.Lock();
			m_ReadBufferList.push_back(newblock);
			m_ReadCacheMutex.Unlock();
			continue;
		}
		else
		{
			newblock.data=diskdata;
			diskdata.resize(0);
			m_ReadCacheMutex.Lock();
			m_ReadBufferList.push_back(newblock);
			m_ReadCacheMutex.Unlock();
			break;
		}
	}

	

	m_ReadCacheMutex.Lock();
	while(GetReadCacheSize() < m_ReadBufferList.size())
	{
		m_ReadBufferList.pop_front();
	}
	m_ReadCacheMutex.Unlock();
	
	return true;


	
}

//only used by check old file when task start
bool CBTStorage::ReadDataWithoutBuffer( std::string& data, int index, unsigned int offset, unsigned int len )
{

    bool bret = ReadDataFromDisk( data, index, 0, GetPieceLength( index ) );
	if(!bret) return false;


	if(offset!=0 || len < data.size())
	{
		data=data.substr(offset,len);
	}

	return true;
}


bool CBTStorage::ReadDataFromDisk( std::string& data, int index, llong offset, unsigned int len )
{

    char* buf = new char[ len ];
//new way

	llong datahead= llong(index) * llong(m_pTorrentFile->GetPieceLength()) + offset;
	llong datarear= llong(len) + datahead;

	llong readpos=datahead;


	SockLib::CAutoLock al(m_FileSystemLock);


	while(readpos < datarear)
	{
		TStorageFileInfo sFileInfo;
		
		if(!GetFileInfoByOffset( sFileInfo, readpos ))
		{
			delete[] buf;
			return false;
		}

#if defined(WINCE)||defined(WIN32)
		if(sFileInfo.handle==INVALID_HANDLE_VALUE)
#else
		if(sFileInfo.handle==-1)
#endif
		{
			//maybe this file is a virtual file ,the data is all zero
			if(sFileInfo.fileInfo.vfile)
			{
				llong fileoffset= readpos - sFileInfo.fileInfo.offset; //ÎÄŒþÄÚµÄÆ«Î»£¬ŽÓÕâÀï¿ª¶Á

				//int rl= MIN(sFileInfo.fileInfo.size - fileoffset, datarear-readpos);
				unsigned int rl=0;
				if(sFileInfo.fileInfo.size - fileoffset > datarear-readpos)
				{
					rl= (unsigned int)(datarear-readpos);
				}
				else
				{
					rl= (unsigned int)(sFileInfo.fileInfo.size - fileoffset);
				}

				memset(buf+(unsigned int)(readpos-datahead),0,rl); //set all to zero, this is a virtual file

				readpos+=rl;
				continue;
			}
			else
			{ //maybe we don't download this file ,so can't read it.
				delete[] buf;
				return false;
			}
		}

		llong fileoffset= readpos - sFileInfo.fileInfo.offset; //ÎÄŒþÄÚµÄÆ«Î»£¬ŽÓÕâÀï¿ª¶Á

		unsigned int nbytes=0;

		if(sFileInfo.fileInfo.size - fileoffset > datarear - readpos)
		{
			nbytes=(unsigned int)(datarear - readpos);
		}
		else
		{
			nbytes=(unsigned int)(sFileInfo.fileInfo.size - fileoffset);
		}

		unsigned int dataoffset= (unsigned int)(readpos-datahead); //ÊýŸÝÆ«ÒÆÎ»ÖÃ


#if defined(WINCE)||defined(WIN32)
		LARGE_INTEGER li;
		li.QuadPart=fileoffset;
		DWORD dwRet=::SetFilePointer(sFileInfo.handle, li.LowPart, &li.HighPart, FILE_BEGIN); 
		if(dwRet==0xFFFFFFFF && ::GetLastError()!=NO_ERROR)
		{
			OutputDebugString(L"Fail to Seek file pos");
			delete[] buf;
			return false;
		}

#else
        llong check1=lseek64( sFileInfo.handle, fileoffset, SEEK_SET ); //ÒÆ¶¯µœÎÄŒþÄÚÆ«ÒÆ
		if(check1!=fileoffset)
		{
			delete[] buf;
			return false;
		}
#endif

#if defined(WINCE)||defined(WIN32)

		DWORD dwRead;
		BOOL bRet=::ReadFile(sFileInfo.handle, buf+dataoffset, nbytes, &dwRead, NULL);
		if(!bRet) {
			OutputDebugString(L"Read file data failed");
			delete[] buf;
			return false;
		}
		readpos+= llong(nbytes);

#else
        int check2 = read( sFileInfo.handle, buf + dataoffset, nbytes );
		if(check2!=nbytes) 
		{
			delete[] buf;
			return false;
		}
		readpos+= llong(check2);
#endif
		
		
	}

	data.resize(0);
	data.append( (const char*) buf, len);

    delete[] buf;
	
    return true;

}

bool CBTStorage::ReadPiece( std::string& data, int index )
{
    return ReadData( data, index, 0, GetPieceLength( index ) );
}

//only use by checkoldfile when start
bool CBTStorage::ReadPieceWithoutBuffer( std::string& data, int index )
{
    return ReadDataWithoutBuffer( data, index, 0, GetPieceLength( index ) );
}

bool CBTStorage::GetFileInfoByOffset(TStorageFileInfo& info, llong offset )
{
    TFileHandleList::iterator it ;
	
    for ( it = m_FileHandleList.begin(); it != m_FileHandleList.end(); it++ )
    {
        if ( it->fileInfo.offset <= offset &&
			it->fileInfo.offset + it->fileInfo.size > offset )
        {
			info=(*it);
            return true;
        }
    }
	
    return  false;

}

float CBTStorage::GetFinishedPercent()
{
    return m_fFinishedPercent;
	
}


llong CBTStorage::GetSelectedCount()
{
    return m_pTorrentFile->GetTotalSize(false /*without virtual files*/) - m_nBanedSize;
}

unsigned int CBTStorage::GetReadCacheSize()
{
	return m_nCacheSize / BUFFER_BLOCK_SIZE; //»ºŽæÖÐÄÜŽæ¶àÉÙžöÊýŸÝ¿é£¿
}


void CBTStorage::GenPriorityBitSet()
{

	m_PriorityBitSet.Init( m_pTorrentFile->GetPieceCount()); //È«0³õÊŒ»¯

    TFileHandleList::iterator it;
	
    for ( it = m_FileHandleList.begin(); it != m_FileHandleList.end(); it++ )
    {
		if(it->fileInfo.vfile)
		{
			continue;
		}

        if ( it->fileInfo.size == 0 )
        {
            continue;
        }

		unsigned int beginIndex= it->headindex;
		unsigned int endIndex=it->tailindex;

		
		if( (it->Priority & 0x00000100) && (it->Priority & 0x000000FF) )
		{//preview mode
			
			//try to split the pieces into 20 segment, the last segment have base priority
			//6,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,1  is the priority add on
			
			//int base=it->Priority & 0x000000FF;
			
			if( endIndex - beginIndex >=20 )
			{
				int base=it->Priority & 0x000000FF;
				int range=endIndex-beginIndex;
				
				//can split into 10 segment
				
				for(unsigned int i=beginIndex; i< endIndex; i++)
				{
					//span from 0 to 19
					int span= int(20*float(i - beginIndex)/float(range)); 
					int rid;
					if(span==0)
					{
						rid=6;
					}
					else if(span >=1 && span <=2)
					{
						rid=5;
					}
					else if(span >=3 && span <=5)
					{
						rid=4;
					}
					else if(span >=6 && span <=9)
					{
						rid=3;
					}
					else if(span >=10 && span <=14)
					{
						rid=2;
					}
					else
					{
						rid=1;
					}
					
					m_PriorityBitSet.SetValue(i, MAX(base + rid , m_PriorityBitSet.GetValue(i)));
				}
			}
			else
			{
				//can't split into 20 segment, no preview mode set
				//little file ,maybe scr file ,so just give it highest priority
				for ( unsigned int i = beginIndex; i < endIndex; i++ )
				{

					m_PriorityBitSet.SetValue(i, MAX((it->Priority & 0x000000FF)+6,m_PriorityBitSet.GetValue(i))); 
				}
			}
		}
		else if( it->Priority & 0x000000FF ) 
		{//not in preview mode 
			for ( unsigned int i = beginIndex; i < endIndex; ++i )
			{
				m_PriorityBitSet.SetValue(i, MAX((it->Priority & 0x000000FF),m_PriorityBitSet.GetValue(i))); 
			}
		}
		else
		{//baned
			for ( unsigned int i = beginIndex; i < endIndex; ++i )
			{

				m_PriorityBitSet.SetValue(i, MAX(0,m_PriorityBitSet.GetValue(i))); 
			}
		}
    }
	
}

int CBTStorage::GetPieceIndexByOffset( llong offset )
{

    int index= (int)(offset / m_pTorrentFile->GetPieceLength());
//the index maybe equal to m_pTorrentFile->GetPieceCount()
//if the offset is at the tail and it's piece aligned
	//assert(index <= m_pTorrentFile->GetPieceCount()); //here have trouble!

	return index;
}


//void CBTStorage::SetDestPath(const char* path)
void CBTStorage::SetDestPath( const wchar_t* path )
{
	
	
    wcscpy( m_sDestPath, path );
    int len = wcslen( m_sDestPath );

#if defined(WINCE)||defined( WIN32)
	
    if ( m_sDestPath[ len - 1 ] != L'\\' )
    {
        wcscat( m_sDestPath, L"\\" );
    }
	
#else
    if ( m_sDestPath[ len - 1 ] != L'/' )
    {
        wcscat( m_sDestPath, L"/" );
    }
	
#endif
}


void CBTStorage::SetCacheSize( unsigned int count )
{
	m_nCacheSize=count*1024;
	if(m_nCacheSize < 5*1024*1024)
	{
		m_nCacheSize=5*1024*1024;
	}

}

BencodeLib::CTorrentFile* CBTStorage::GetTorrentFile()
{
    return m_pTorrentFile;
}


bool CBTStorage::IsDownloadingPiece( int index )
{
	SockLib::CAutoLock al(m_DownloadingMapMutex);
	return m_DownloadingMap.find(index)!=m_DownloadingMap.end();
}

bool CBTStorage::IsFinishedPiece( int index )
{
    return m_FinishedBitSet.IsSet( (unsigned int)index );
}

//initially build filelist

void CBTStorage::BuildFileListStructure()
{

    m_nBanedSize = 0;
	
	std::string mainname=m_pTorrentFile->GetName();

#if defined( WIN32)||defined(WINCE)
	mainname+="\\";
#else
	mainname+="/";
#endif

    unsigned int count = m_pTorrentFile->GetFileNumber(true /*get all files*/);
	

	int vircount=0; 

    for ( unsigned int i = 0; i < count; i++ )
    {
        TStorageFileInfo sfi;
        sfi.fileInfo = m_pTorrentFile->GetFileInfo( i, true /*get all files*/ );

#if defined(WINCE)||defined(WIN32)
		sfi.handle=INVALID_HANDLE_VALUE;
#else
        sfi.handle = -1;
#endif


		sfi.headindex=GetPieceIndexByOffset(sfi.fileInfo.offset);

		sfi.tailindex=GetPieceIndexByOffset(sfi.fileInfo.offset+sfi.fileInfo.size);

		if((sfi.fileInfo.offset+sfi.fileInfo.size)%m_pTorrentFile->GetPieceLength())
		{//the file not align to the piece edge
			sfi.tailindex+=1;
		}


		if(sfi.fileInfo.vfile)
		{
			sfi.Priority = 0; 
#if defined(WINCE)||defined(WIN32)
			sfi.handle = INVALID_HANDLE_VALUE;
#else
			sfi.handle = -1;
#endif
			vircount++;

			//LogMsg( L"have virtual file", 0, MSG_INFO );					

		}
		else
		{
			//set default file priority
			sfi.Priority = GetFilePriority( i - vircount );

			if(IsFileInPreviewMode( i - vircount )) { sfi.Priority|=0x00000100 ;}

			if ( (sfi.Priority & 0x000000FF) == 0 )
			{
				m_nBanedSize += sfi.fileInfo.size;
			
			}

			if(!m_pTorrentFile->IsSingleFile())
			{
				sfi.fileInfo.name=mainname+sfi.fileInfo.name;
			}

		}

        m_FileHandleList.push_back( sfi );
    }
	

	m_PieceInVirtualFile.clear();

	TFileHandleList::const_iterator it;
	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile)
		{

			int beginIndex = GetPieceIndexByOffset( it->fileInfo.offset );
			//endIndex is not included 
			int endIndex = GetPieceIndexByOffset( it->fileInfo.offset + it->fileInfo.size );

			if(( it->fileInfo.offset + it->fileInfo.size )%m_pTorrentFile->GetPieceLength())
			{	
				endIndex+=1;
			}

			llong vfilebegin= it->fileInfo.offset;
			llong vfileend= it->fileInfo.offset + it->fileInfo.size;


			for(int i=beginIndex; i< endIndex;i++)
			{
				llong piecebegin= llong(i) * llong(m_pTorrentFile->GetPieceLength());
				int piecelen= GetPieceLength(i);
				llong pieceend= piecebegin + llong(piecelen);

				if(vfilebegin <= piecebegin && piecebegin <= vfileend )
				{

					unsigned int voff= 0; 
					unsigned int vlen= (pieceend > vfileend)? (unsigned int)(vfileend - piecebegin):(piecelen);

					if(vlen >0 )
					{
						TVirtualAffect vf;
						vf.offset=voff;
						vf.length=vlen;
						m_PieceInVirtualFile[i]=vf;

//						wchar_t msg[128];
//						swprintf(msg,L"piece=%u, voff=%u, vlen=%u, piecelen=%u", i, voff,vlen, m_pTorrentFile->GetPieceLength());
//						LogMsg(msg,0,MSG_INFO);
					}

				}
				else if(vfilebegin >= piecebegin && vfilebegin <= pieceend ) 
				{
					unsigned int voff= (unsigned int)(vfilebegin - piecebegin);
					unsigned int vlen= (pieceend > vfileend)? (unsigned int)(vfileend - vfilebegin): (unsigned int)(piecelen-voff);

					if(vlen >0 )
					{
						TVirtualAffect vf;
						vf.offset=voff;
						vf.length=vlen;
						m_PieceInVirtualFile[i]=vf;
					}

				}
				else
				{
					//ÆäËûÇé¿ö²»ÖØµþ£¬ºöÂÔ
				}

			}

		}
	}

}



//diff from buildfileliststructure() for the file is opened
//just adjust the priority if new file selected open it
//if unselected ,close it; if only priority change, change it
//Ò»¶ÔÒ»Æ¥ÅäÊµÎÄŒþ£¬UI²ãÃæ¿Ž²»µœÐéÎÄŒþ£¬ËùÒÔÒª±Ü¿ªÐéÎÄŒþÀŽÉèÖÃÓÅÏÈŒ°

bool CBTStorage::RebuildFileListStructure()
{
	
    TFileHandleList::iterator it;

	bool newopen=false;

	int vircount=0; //ÐéÎÄŒþŒÇÊý

	m_nBanedSize=0;

    for ( it = m_FileHandleList.begin();it != m_FileHandleList.end();it++ )
    {
		if(it->fileInfo.vfile) 
		{
			vircount++;
			continue;
		}

		it->Priority = GetFilePriority( it->fileInfo.index - vircount );
		if(IsFileInPreviewMode(it->fileInfo.index - vircount )) { it->Priority|=0x00000100 ;}

		if ( (it->Priority & 0x000000FF)==0 )
		{
			m_nBanedSize += it->fileInfo.size; 
			//don't close the originally opened handle
			//because the data may help us download 
		}
		else
		{
#if defined(WINCE)||defined(WIN32)
			if(it->handle==INVALID_HANDLE_VALUE) {
#else
			if(it->handle==-1) {
#endif
				newopen=true;
				OpenSingleFile( *it ); //need to open new file for write
			}
		}
	}	

	return newopen;
}

//work based on m_FileHandleList
//call buildfileliststructure() first
bool CBTStorage::GetFilesReady()
{
    TFileHandleList::iterator it;
	
    for ( it = m_FileHandleList.begin();it != m_FileHandleList.end();it++ )
    {
        if ( it->fileInfo.vfile )
			continue;
		else if(!OpenSingleFile( *it ) )
            return false;
    }
	
    return true;
}



void CBTStorage::CalculateAvailability()
{ 
	//new cal, avialability is xx.yy
	//xx indicate all full copy number
	//yy indicate the cover without full copy
	
	//scan piecesum two time, first is find the smallest value, it's the xx
	//second find how many piece value > xx ,it's the yy;
	unsigned int count = m_bitPieceSum.GetSize();
	unsigned int i;
	unsigned int xx=0x0FFFFFFF;
	
    for ( i = 0;i < count;i++ )
    {
        unsigned int val = m_bitPieceSum.GetValue( i );
		xx=MIN(val,xx); //find the smallest value.
    }	
	
	//then find the number of piece that greater than xx;
	unsigned int great=0;
	for ( i=0;i<count;i++)
	{
		if((unsigned int)(m_bitPieceSum.GetValue( i )) > xx)
		{
			great++;
		}
	}
	
	float yy=float(great)/float(count);
	m_fPieceAvailability=float(xx)+yy;
	
}


bool CBTStorage::Convert(const char* multibyte, int nbytes, UINT codepage, wchar_t* wpBuf, int wbuflen)
{
	int n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, wbuflen);
	if(n>0) wpBuf[n]=0;	//安全问题
	return n>0;
}

//if open ok, the handle should write back to info
bool CBTStorage::OpenSingleFile( TStorageFileInfo& info )
{

	if(info.fileInfo.vfile) return true;  //ÐéÄâÎÄŒþ²»ÓÃŽò¿ª

    wchar_t filePath[ MAX_PATH ] = {0};
	
	//准备UNICODE文件名
    if ( m_pTorrentFile->IsUtf8Valid() )
    {
        wchar_t buf[ MAX_PATH ];
        Tools::UTF2UCS( ( char* ) ( info.fileInfo.name.c_str() ), buf, MAX_PATH );

        wcscpy( filePath, m_sDestPath );
        wcscat( filePath, buf );
    }
    else
    {

		wchar_t buf[ MAX_PATH ];
		if(!Convert(info.fileInfo.name.data(), info.fileInfo.name.size(), m_CodePage, buf, MAX_PATH))
		{
			m_JobStatus=_JOB_FAILED;
			return false; //暂时不处理非utf8格式的种子
		}


        wcscpy( filePath, m_sDestPath );
        wcscat( filePath, buf );		

    }
	

    if ( info.Priority && info.handle == INVALID_HANDLE_VALUE )
    {
		
		//make sure the file path exists!
		MakeSureDirectoryExistsForFile(filePath);

		HANDLE handle=INVALID_HANDLE_VALUE;

		//先尝试打开旧文件
		handle=::CreateFile(filePath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(handle!=INVALID_HANDLE_VALUE)
		{
			//检查文件大小
			LARGE_INTEGER li;
			li.LowPart=::GetFileSize(handle, (DWORD*)&li.HighPart);
			if(li.LowPart==0xFFFFFFFF && GetLastError() != NO_ERROR)
			{
				//error!
				::CloseHandle(handle);
				handle=INVALID_HANDLE_VALUE;
			}
			else
			{
				//no error for file length
				if(li.QuadPart==info.fileInfo.size)
				{
					m_bOldFileExists=true;
					info.handle=handle;
				}
				else
				{
					//文件大小不同, 无效打开
					::CloseHandle(handle);
					handle=INVALID_HANDLE_VALUE;
				}
			}

		}

		if(handle==INVALID_HANDLE_VALUE)
		{

			//上面打开旧文件失败了, 创建一个新文件, 如果有老文件替换
			handle = ::CreateFile( filePath, GENERIC_READ|GENERIC_WRITE, 
				FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if(handle==INVALID_HANDLE_VALUE)
			{
                return false;
			}

			LARGE_INTEGER li;
			li.QuadPart=info.fileInfo.size;
			DWORD fp;
			fp=::SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
			if(fp==0xFFFFFFFF && GetLastError!=NO_ERROR)
			{
				//failed.
				::CloseHandle(handle);
				return false;
			}

			if(!::SetEndOfFile(handle))
			{

				::CloseHandle(handle);
				return false;
			}

			::SetFilePointer(handle, 0, NULL, FILE_BEGIN);
			info.handle=handle;

			return true;

		}

		
    }
    else
    {
		
		//although we skipped this file, but ,if the file exists and have some data
		//then open it will help us download faster
		info.handle=::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(info.handle!=INVALID_HANDLE_VALUE) m_bOldFileExists = true;

		////check if the file exists?
		//if ( CFileWrap::GetFileLength( filePath ) == info.fileInfo.size )
		//{		//if exists open it
  //          //length is equal, so need check the sha1
  //          m_bOldFileExists = true;

		//	info.handle = ::CreateFile( filePath, GENERIC_READ|GENERIC_WRITE, 
		//		FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);

		//}
		//else
		//{
		//	//file does not exists and we don't download it, skip.
		//	OutputDebugString(L"Skip a file that not interested and not exists");
		//	
		//}
		
    }
	
    return true;
}

void CBTStorage::InitFilePriority( std::string& prios )
{
	m_FilePriority=prios;
}

float CBTStorage::GetAvailability()
{
    return m_fPieceAvailability;
}

void CBTStorage::PieceChangeNotice( CBTPiece& bitset, bool newpeer )
{
	SockLib::CAutoLock al(m_bitPieceSumMutex);
	
    if ( newpeer )
        m_bitPieceSum += bitset;
    else
        m_bitPieceSum -= bitset;
	
    m_bPieceSumChanged = true;
	
    CalculateAvailability();

}

//¿Ž¿ŽÀÏŽúÂë£¬ÕâÀïºÃÏóÓÐÎÊÌâ
void CBTStorage::PeerHaveNewPieceNotice(unsigned int iip, int index )
{
	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());
	
	m_PeerCenter.PeerHaveNewPieceNotice(iip,index);
	
	SockLib::CAutoLock al(m_bitPieceSumMutex);
	
	m_bitPieceSum.NewPiece( index );
	
	m_bPieceSumChanged = true;	
	
	CalculateAvailability();
	
	

}

void CBTStorage::SumUpDownload( unsigned int iip, int count )
{
    m_nSumOfDownload += count;
	m_PeerCenter.GotChunk(iip,count/1024/16);

}

void CBTStorage::SumUpUpload( unsigned int iip, int count )
{
    m_nSumOfUpload += count;
	m_PeerCenter.SendChunk(iip,count/1024/16);

}

llong CBTStorage::GetSumOfDownload()
{
    return m_nSumOfDownload;
}

llong CBTStorage::GetSumOfUpload()
{
    return m_nSumOfUpload;
}

//call by session
unsigned int CBTStorage::RegisteSession( CBTSession* sess )
{
	SockLib::CAutoLock al(m_SessionListMutex);

    m_SessionList.push_back( sess );

	//Íš¹ýÊµÑé·¢ÏÖ£¬Ò»Ð©ÖÖ×Ó·ŽžÐÒ»žöµØÖ·µÄ¶àÁ¬œÓ£¬µŒÖÂºóÀŽÁ¬²»ÉÏÖÖ×Ó
	//ËùÒÔ»¹ÊÇ¿ŒÂÇ¶àžöPEERIDµÄ·œ°ž£¬»òÐížüÎÈÍ×

	if(m_bIsTaskFinished) sess->DownloadFinish();
	
	return m_SessionIdCounter++;

}


//get a pointed piece job
bool CBTStorage::GetPieceTask( int index , bool& coorperate)
{
#ifdef _CHECK
	LogMsg(L"try to get appointed piece",0, MSG_INFO);
#endif

	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	//check if this piece finished
    if ( m_FinishedBitSet.IsSet( index ) )
    {
#ifdef _CHECK
		LogMsg(L"this piece alread finished",0,MSG_INFO);
#endif
		return false;
	}
	
	//check if this piece baned
	if ( m_PriorityBitSet.GetValue(index) == 0 )
	{
#ifdef _CHECK
		LogMsg(L"this piece be baned",0,MSG_INFO);
#endif
		return false;
	}
	
    //if can't find a task piece ,find in downloading list
  
	if(IsDownloadingPiece(index))
	{
		SockLib::CAutoLock al(m_DownloadingMapMutex);

		if(m_PriorityBitSet.GetValue(index) > 10 && m_DownloadingMap[index] <=2 )
		{
			coorperate=true;
			m_DownloadingMap[index]++;
			return true;
		}
		else if(m_PriorityBitSet.GetValue(index) > 5 && m_DownloadingMap[index] <=1 )
		{
			coorperate=true;
			m_DownloadingMap[index]++;
			return true;
		}

		return false;
	}
	else
	{
		m_DownloadingMap[index]=1;
		
		if(m_PriorityBitSet.GetValue(index) > 5)
		{
			coorperate=true;
		}
		else
		{
			coorperate=false;
		}

		return true;
	}

}

bool CBTStorage::IsPieceFinish( int index )
{
	assert(index>=0);
	assert(index < m_pTorrentFile->GetPieceCount());

    return m_FinishedBitSet.IsSet( (unsigned int)index );
}

//test if we interest this piece
bool CBTStorage::IsPieceInterest( int index )
{
	assert(index>=0);
	assert(index < m_pTorrentFile->GetPieceCount());

    if ( m_FinishedBitSet.IsSet( index ) )
    {
		return false;  //finished
	}

    if ( m_PriorityBitSet.GetValue( index ) == 0 )
    {
		return false; //be baned
	}
	

	
    return true; //we interest!
}

//test if we interest this bitset
unsigned int CBTStorage::IsPieceInterest( CBTPiece& bitSet )
{
    //make a copy of m_PriorityBitSet to use
    CBTPieceSum pc = m_PriorityBitSet;
	
	unsigned int i;
	unsigned int tsize=pc.GetSize();
	
    //mark off peer's bitset
    for ( i = 0; i < tsize; i++ )
    {
        if ( !bitSet.IsSet( i ) )
        {
			pc.SetValue( i, 0 );
		}
    }
	
    //mark off finished pieces
    for ( i = 0; i < tsize; i++ )
    {
        if ( m_FinishedBitSet.IsSet( i ) )
		{
			pc.SetValue( i, 0 );
		}
    }
	

	return pc.GetSetCount();

}

//this function is as a replace of IsPieceInterest


//test if this peer's bitset will interest me
//any bitset we have and peer don't have?
bool CBTStorage::IsPeerNeedMyPiece( CBTPiece& bitSet )
{
	
	unsigned int i;
	unsigned int tsize=m_FinishedBitSet.GetSize();
	
    //mark off finished pieces
    for ( i = 0; i < tsize; i++ )
    {
        if ( m_FinishedBitSet.IsSet( i ) && !bitSet.IsSet( i ) )
		{
			return true;
		}
    }
	
    return false;
}

void CBTStorage::UnregisteSession( CBTSession* sess )
{

	SockLib::CAutoLock al(m_SessionListMutex);

    TSessionList::iterator it;
	
    for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
    {
        if ( (*it) == sess )
        {
            m_SessionList.erase( it );
            break;
        }
    }
	
}

//for every session
unsigned int CBTStorage::GetLinkMax()
{
    return (unsigned int)(m_nConnectionLinkMax*m_fConnectionRatio/SESSIONNUM);
}

//for every session
unsigned int CBTStorage::GetUploadLinkMax()
{


	//this is for every sessions
	//we calculate from the upload speed limit by myself now
	
	//limit by upload speed limit and limit by cache size
	//
//*/ new way

	//µ±ÎÒÃÇ»¹ÔÚÏÂÔØÊ±£¬Ÿ¡Á¿¶à¿ªÉÏŽ«Á¬œÓ£¬Íê³Éºó¿ÉÒÔÉÙ¿ªµã
	if(IsFinished())
	{
		//ÎªÃ¿žöÉÏŽ«Á¬œÓ±£Ö€6žö¿é£š6*128k£©Î»ÖÃ,ŒõÉÙ³ÖÐøÉÏŽ«ÊýŸÝ¶ÔÓ²ÅÌµÄ³å»÷
		//15M»ºŽæÊÇ20žöÁ¬œÓÎ»
		unsigned int cachelimit=GetReadCacheSize()/6;

		unsigned int speedlimit;

		speedlimit=m_UploadSpeedLimit/8/1024;

		unsigned int minup=MIN(cachelimit,speedlimit);

		if( minup < SESSIONNUM ) return 1; //at least every session have 1 upload link
		else return minup/SESSIONNUM;
	}
	else
	{
		//Ã¿žöÉÏŽ«±£³Ö 128k Î»ÖÃ, 1žö»ºŽæ¿é
		unsigned int cachelimit=GetReadCacheSize();	//40 if cache = 10M //ÎªÃ¿žöÉÏŽ«Á¬œÓ±£Ö€1žö¿éÎ»ÖÃ

		unsigned int speedlimit;
		speedlimit=m_UploadSpeedLimit/3; //every link have at least 3k/s speed
		

		unsigned int minup=MIN(cachelimit,speedlimit);

		if( minup < SESSIONNUM ) return 1; //at least every session have 1 upload link
		else return minup/SESSIONNUM;
	}


	/*/
	int ratio=1;
	
	if(m_UploadSpeedLimit < 5*1024)
	{
		return 1*ratio;
	}
	else if(m_UploadSpeedLimit < 10*1024)
	{
		return 2*ratio;
	}
	else if(m_UploadSpeedLimit < 20*1024)
	{
		return 4*ratio;
	}
	else if(m_UploadSpeedLimit < 50*1024)
	{
		return 8*ratio;
	}
	else if(m_UploadSpeedLimit < 100*1024)
	{
		return 16*ratio;
	}
	else if(m_UploadSpeedLimit < 200*1024)
	{
		return 32*ratio;
	}
	else
	{
		return 40*ratio; //the max!
	}
	//*/
}


bool CBTStorage::IsPieceSumChanged()
{
    return m_bPieceSumChanged;
}

bool CBTStorage::IsPriorityChanged()
{
    return m_bPriorityChanged;
}

int CBTStorage::GetAllPieceNumber()
{
    return m_pTorrentFile->GetPieceCount();
}

int CBTStorage::GetFinishedPieceSet( int* buf )
{
	//Ñ¹ËõÁ÷
	memcpy(buf, m_FinishedBitSet.GetStream().data(), m_FinishedBitSet.GetStream().size());
	
    //m_bFinishedPieceChanged = false;

	return m_FinishedBitSet.GetStream().size();
}

void CBTStorage::GetSumPieceSet( int *buf )
{
	
	unsigned int tsize = m_bitPieceSum.GetSize();
	
	for ( unsigned int i = 0; i < tsize; i++ )
	{
		buf[ i ] = m_bitPieceSum.GetValue( i );
	}
	
	m_bPieceSumChanged = false;

}

void CBTStorage::GetPrioritySet( int* buf )
{
	unsigned int tsize = m_PriorityBitSet.GetSize();
	
    for ( unsigned int i = 0;i < tsize; i++ )
    {
        buf[ i ] = m_PriorityBitSet.GetValue( i );
    }
	
    m_bPriorityChanged = false;
}

float CBTStorage::CalculateFinishedPercent()
{
    unsigned int allsize = m_FinishedBitSet.GetSize();
	
    unsigned int seted = 0;
    unsigned int unseted = 0;
	
    for ( unsigned int i = 0;i < allsize;i++ )
    {
        if ( m_PriorityBitSet.GetValue( i ) )
        {
            if ( m_FinishedBitSet.IsSet( i ) )
                seted++;
            else
                unseted++;
        }
    }
	
    if ( seted + unseted == 0 )
        return 1.0f;
    else
        return float( seted ) / float( seted + unseted );
}

void CBTStorage::SetParent( CBTJob* parent )
{
    m_pParent = parent;
}

void CBTStorage::CheckIsTaskFinished()
{
    //GetUnsetCount return the baned count now
    //m_PriorityBitSet will be dynamic change,so check through
    //return _finishedPiece == (m_FinishedBitSet.GetSize() - m_PriorityBitSet.GetUnsetCount());
    unsigned int count = m_FinishedBitSet.GetSize();
	
    for ( unsigned int i = 0;i < count;i++ )
    {
        if ( m_PriorityBitSet.GetValue( i ) && !m_FinishedBitSet.IsSet( i ) )
        {
            m_bIsTaskFinished = false;
            return ;
        }
    }
	
    m_bIsTaskFinished = true;
    return ;
}


//broadcast to all sessions that we have finished this piece
void CBTStorage::BroadcastHavePiece( int index )
{
	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());
	
	SockLib::CAutoLock al(m_SessionListMutex);

    TSessionList::const_iterator it;

    for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
    {
        ( *it ) ->BroadcastNewPiece( index );
    }

}

//void CBTStorage::SetChkDirCallback( CREATEDIR cb )
//{
//    m_pChkDirCallback = cb;
//}

//void CBTStorage::SetMbConvCallback( MB2WCCONV cb )
//{
//    m_pMBCallback = cb;
//	
//}

unsigned int CBTStorage::GetConnectingMax()
{
	return (unsigned int)(m_nConnectingLinkMax*m_fConnectingRatio/SESSIONNUM );
}

void CBTStorage::SetConnectingLinkMax(unsigned int con)
{
	
	m_nConnectingLinkMax=con; //>400?400:con;

	unsigned int cons= (unsigned int)(m_nConnectingLinkMax*m_fConnectingRatio);
	if(cons > 100) cons=100;
	else if(cons < 10) cons=10;

	//cons-= 3; //tracker and torrent share.

	m_PeerCenter.SetConnectingLimit(cons); //ÔÚwin2000ÏÂ²âÊÔ£¬Á¬œÓ·¢ÆðÌ«Žó²¢²»ºÃ


}

//replace GetShareRequestBlock
bool CBTStorage::GetShareTask(int index, unsigned int &offset,unsigned int& length)
{
	assert(index >=0);
	assert(index <m_pTorrentFile->GetPieceCount());

	if(IsFinishedPiece(index)) 
	{
		//LogMsg(L"share task have finished!",0,MSG_INFO);
		return false;
	}

	SockLib::CAutoLock al(m_ShareRequestMapMutex);

	TShareRequestMap::iterator it;

	it=m_ShareRequestMap.find(index);

	if(it!=m_ShareRequestMap.end())
	{
		//bool bret=it->second.GetTask(offset,length,10);
		//if(!bret) {
			//LogMsg(L"share task get fail-1\n",0,MSG_INFO);
			//if(it->second.IsFinish()) LogMsg(L"this piece have finished",0,MSG_ERROR);
			//else LogMsg(L"this piece have not finished",0,MSG_ERROR);
		//}
		//return bret;
		return it->second.GetTask(offset,length,10);
	}
	else
	{
		CShareRequest request;
		request.Init(index, GetPieceLength(index), m_PriorityBitSet.GetValue(index));
 		
		unsigned int voff, vlen;
 		if(GetAffectRangeByVirtualFileInPiece(index, voff, vlen))
 		{
 			request.SetVirtualData(voff,vlen);
 		}

		//check out orphan data for share task
		std::list<COrphan> orphans;
		std::list<COrphan>::iterator it;

		CheckOutOrphanData(index,orphans);

		for(it=orphans.begin(); it!=orphans.end();it++)
		{
			request.SetData(it->source,it->offset,it->data);
		}

		if(request.IsFinish())
		{

			std::string piecedata=request.GetPieceData();
		
			if( Tools::SHA1String ( piecedata )
				== GetPieceHash( index ) )
			{
				
				WritePiece( index, piecedata );
				//LogMsg(L"share task get fail-2\n",0,MSG_INFO);
				return false;
			}
			else
			{
				request.Init(index, GetPieceLength(index), m_PriorityBitSet.GetValue(index));
				m_ShareRequestMap[index]=request;
				//bool bret=m_ShareRequestMap[index].GetTask(offset, length,10);
				//if(!bret) LogMsg(L"share task get fail-3\n",0,MSG_INFO);
				//return bret;
				return m_ShareRequestMap[index].GetTask(offset, length,10);
			}

		}
		else
		{
			m_ShareRequestMap[index]=request;

			//bool bret=m_ShareRequestMap[index].GetTask(offset, length,10);
			//if(!bret) LogMsg(L"share task get fail-4\n",0,MSG_INFO);
			//return bret;			
			return m_ShareRequestMap[index].GetTask(offset, length,10);
		}
	}
}


//replace SubmitCoorperateBlockData
//·µ»ØfalseÔòÈÎÎñ²»±ØŒÌÐø
bool CBTStorage::SubmitShareData(unsigned int source, int index, unsigned int offset, std::string& data)
{

	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	if(IsFinishedPiece(index)) return false;

	SockLib::CAutoLock al(m_ShareRequestMapMutex);

	TShareRequestMap::iterator it;

	it=m_ShareRequestMap.find(index);

	if(it==m_ShareRequestMap.end()) return false;

	it->second.SetData(source,offset,data);

	if(!it->second.IsFinish()) return true;

	//std::string piecedata=it->second.GetPieceData();
	std::string hash=GetPieceHash(index);

	if(it->second.CheckHash(hash))
	{
		std::string piecedata=it->second.GetPieceData();

		WritePiece(index,piecedata);
		//erase this 
		m_ShareRequestMap.erase(index);
		return false;
	}
	else
	{
		//checkhash auto increase the fail time

		it->second.ClearComfirmData();
		return true;
	}

}

//void CBTStorage::SetBTKad(BTKADSERVICE ks)
//{
//	m_pBtKadService=ks;
//}


int CBTStorage::GetListenPort()
{
	return m_nListenPort;
}

bool CBTStorage::AddNewPeer( unsigned int iip, unsigned short iport)
{
	return m_PeerCenter.AddAvialablePeerInfo(iip,iport);
}


//the peers got DHT node from bittorrent protocol notice
//directly add to callback service
void CBTStorage::NewDHTNode(unsigned int iip, unsigned short iport)
{
	//从BT下载过来的引导节点这回没有可能传递到DHT里面了,因为没有回调函数

	//if(m_pBtKadService!=NULL)
	//{
	//	m_pBtKadService(_ADD_NODE,NULL,NULL,iip,(int*)(&iport));
	//}
}


//this thread is for listener
void CBTStorage::Entry()
{
	unsigned int lastwrite=0;
	unsigned int lastcalspeed=0;
	llong lastdownloadsum=0;
	llong lastuploadsum=0;
	

	m_UpByteCount=m_UploadSpeedLimit;
	m_DownByteCount=m_DownloadSpeedLimit;

//	unsigned int lastmyid=GetTickCount();	//±ä»¯ID
	unsigned int lastprog=GetTickCount(); //Œì²éœø¶È
	unsigned int progcount=0;		//Œì²éœø¶ÈµÄŒÇÂŒÊý
	//originally in start() because check old files will takes much time
	//I will to move to here to avoid the gui no response	
	JobInit(); 

	//ÎÄŒþŒì²éÍê³É£¬×¢²áÈÎÎñ
	//std::string hash;
	//hash.append(m_pTorrentFile->GetInfoHash(),20);
	m_pSingleListener->RegisteTask(m_pTorrentFile->GetInfoHash(),this);

	unsigned int lasttick=GetTickCount();

	while(!m_bStop)
	{

		Sleep(100);

		//calculate up/down limit bytes
		unsigned int thistick = GetTickCount();

		if(thistick < lasttick) 
		{
			lasttick=thistick;
		}

		if ( thistick - lasttick > 500 )
		{
			m_UpByteCount += int((float(thistick - lasttick) / 1000.0f)* m_UploadSpeedLimit);
			if(m_UpByteCount > m_UploadSpeedLimit) m_UpByteCount=m_UploadSpeedLimit;

			m_DownByteCount += int((float(thistick - lasttick) / 1000.0f)* m_DownloadSpeedLimit);
			if(m_DownByteCount > m_DownloadSpeedLimit) m_DownByteCount=m_DownloadSpeedLimit;

			//global speed cal
			llong thisdown=m_nSumOfDownload - lastdownloadsum;
			llong thisup  =m_nSumOfUpload - lastuploadsum;
			
			int speeddown= int(thisdown*1000/(thistick-lasttick));
			int speedup  = int(thisup*1000/(thistick-lasttick));

			lasttick = thistick;
			
			m_SpeedHistoryList.push_back(speeddown);
			m_UpSpeedHistoryList.push_back(speedup);
			
			lastdownloadsum=m_nSumOfDownload;
			lastuploadsum= m_nSumOfUpload;
			
			while(m_SpeedHistoryList.size() > 10)
				m_SpeedHistoryList.pop_front();
			
			while(m_UpSpeedHistoryList.size() > 10)
				m_UpSpeedHistoryList.pop_front();
			
			//cal ave speed
			int sumspeed=0;
			TIntList::const_iterator it3;
			for( it3=m_SpeedHistoryList.begin();it3!=m_SpeedHistoryList.end();it3++)
			{
				sumspeed+=(*it3);
			}
			
			m_nAverageSpeed= int(float(sumspeed)/float(m_SpeedHistoryList.size()));
			
			sumspeed=0;
			for( it3=m_UpSpeedHistoryList.begin();it3!=m_UpSpeedHistoryList.end();it3++)
			{
				sumspeed+=(*it3);
			}
			
			m_nAverageUpSpeed= int(float(sumspeed)/float(m_UpSpeedHistoryList.size()));
			
			if(m_nAverageSpeed >0)
			{
				m_nTimeToFinish=
					int(float(GetLeftPieceCount())*float(m_pTorrentFile->GetPieceLength())/m_nAverageSpeed); 
			}
			else
			{
				m_nTimeToFinish=-1;
			}
			

		}
		
		if( thistick - lastwrite > 1000)
		{
			
			int nret=SmartWriteCache(); //Œì²é·µ»ØÖµ£¬Èç¹ûÊ§°ÜÔòÍË³öÈÎÎñ
			if(nret>0 && m_WriteCache.empty())
			{
				//ÐŽÈëÁË¶«Î÷£¬²¢ÇÒ»ºŽæÎª¿Õ£¬ŒÇÂŒÆ¬
				//SaveIndex(); 
				//保存交给上层去做了.
			}

			//if(nret<0)
			//{
			//	LogMsg(L"SmartWriteCache return error, Maybe file broken!",m_nTaskId,MSG_ERROR);
			//}

			lastwrite=thistick;

		}

		if( thistick - lastcalspeed > 3000)
		{//calculate the average speed of every peers
			if(lastcalspeed)
			{
				m_PeerCenter.TickForCalSpeed();
			}
			lastcalspeed=thistick;
		}


	}//while(!m_bStop)
	
	m_pSingleListener->UnregisteTask(m_pTorrentFile->GetInfoHash());

	//make sure all data in write cache are write out
	
	TPieceMap swap;		
	m_WriteCacheMutex.Lock();
	swap.swap(m_WriteCache);  //copy all data to our temp, release the lock asap
	m_WriteCacheMutex.Unlock();

	TPieceMap::iterator it;
	for ( it = swap.begin(); it != swap.end(); it++ )
	{
		//ÍË³öÊ±ÐŽÈë£¬²»ÓÃŒì²é·µ»ØÁË¡£
		WritePieceToDisk( it->first, it->second );
	}
	
	
	//SaveIndex();  交给上层去做


}


int CBTStorage::GetFinishedPieceCount()
{
	return m_FinishedBitSet.GetSetedCount();
}

unsigned int CBTStorage::GetLeftPieceCount()
{
    unsigned int allsize = m_FinishedBitSet.GetSize();
	
    unsigned int seted = 0;
    unsigned int unseted = 0;
	
    for ( unsigned int i = 0;i < allsize;i++ )
    {
        if ( m_PriorityBitSet.GetValue( i ) )
        {
            if ( m_FinishedBitSet.IsSet( i ) )
                seted++;
            else
                unseted++;
        }
    }
	
	return unseted;
}

int CBTStorage::GetUploadSpeed()
{
	return m_nAverageUpSpeed;
}

int CBTStorage::GetDownloadSpeed()
{
	return m_nAverageSpeed;
}

int CBTStorage::GetTimeToFinish()
{
	return m_nTimeToFinish;
}

CTrackerCenter* CBTStorage::GetTrackerCenter()
{
	return m_pTrackerCenter;
}

bool CBTStorage::IsSelfPeerId(std::string& peerid)
{
#ifdef _SINGLEID
	return (memcmp((void*)(peerid.data()),m_MyId,20)==0);
#else
	for(int i=0;i<SESSIONNUM;i++)
	{
		if(memcmp(peerid.data(),m_MyId+i*20,20)==0) return true;
	}

	return false;
#endif
}

void CBTStorage::SetConnectionLinkMax(unsigned int con)
{
	m_nConnectionLinkMax=con;

	m_PeerCenter.SetConnectionLimit((unsigned int)(m_nConnectionLinkMax*m_fConnectionRatio));

#ifdef _CHECK
	wchar_t test[128];
#ifdef WIN32	
	swprintf(test,L"connection max set to %d",m_nConnectionLinkMax);
#else
	swprintf(test,128,L"connection max set to %d",m_nConnectionLinkMax);	
#endif
	LogMsg(test,0,MSG_INFO);
#endif
}


void CBTStorage::SetUploadSpeedLimit(int speed)
{
    if ( speed <= 0 )
        m_UploadSpeedLimit = 0x0FFFFFFF;
    else
        m_UploadSpeedLimit = speed*1024;

}

void CBTStorage::SetDownloadSpeedLimit(int speed)
{
    if ( speed <= 0 )
        m_DownloadSpeedLimit = 0x0FFFFFFF;
    else
        m_DownloadSpeedLimit = speed*1024;
	
}

//get how much bytes we can use for upload
int CBTStorage::GetLeftUpBytes()
{
	return MIN(m_UpByteCount, m_pSingleListener->GetLeftUpBytes());
}
//get how much bytes we can use for download
int CBTStorage::GetLeftDownBytes()
{
	return MIN(m_DownByteCount,m_pSingleListener->GetLeftDownBytes());
}

int CBTStorage::RunOffUpBytes(int bytes)
{

	if(bytes<0) return 0;

	m_UpByteCount-=bytes;

	int left=m_pSingleListener->RunOffUpBytes(bytes);
	return MIN(left, m_UpByteCount);

}

int CBTStorage::RunOffDownBytes(int bytes)
{//count down the down bytes

	if(bytes<0) return 0;

	m_DownByteCount-=bytes;

	int left=m_pSingleListener->RunOffDownBytes(bytes);
	return MIN(m_DownByteCount, left);

}

//void CBTStorage::SetEventBack(int taskid, BTTASKEVENT eventback)
//{
//	m_nTaskId=taskid;
//	m_pTaskEvent=eventback; //this event is for instant up/down data count
//	m_PeerCenter.SetEventBack(taskid,eventback);
//}

bool CBTStorage::ReadTorrentContent(std::string& content, UINT codepage)
{
    if ( m_pTorrentFile != NULL )
    {
        delete m_pTorrentFile;
    }
	
    m_pTorrentFile = new BencodeLib::CTorrentFile();
	
    if ( 0 != m_pTorrentFile->ReadBuf( (char*)(content.data()), content.size() ) )
    {
        //LogMsg( L"Read torrent file fail.", 0,MSG_ERROR );
        return false;
    }
	
    if ( 0 != m_pTorrentFile->ExtractKeys() )
    {
        //LogMsg( L"invalid torrent file", 0,MSG_ERROR );
        return false;
    }

	m_CodePage=codepage;
	
    //if ( encode != NULL && wcslen( encode ) <= 15 )
    //{
    //   // wcscpy( m_sEncode, encode ); //ensure wcslen(encode) < 15
    //}
    //else
    //{
    //    //m_sEncode[ 0 ] = wchar_t( 0 );
    //}
	
	//check hash
    return true;
}

void CBTStorage::SetInitBitSet(std::string& bitset)
{
	m_InitBitSet=bitset;
}

//void CBTStorage::SetPieceBack(BTTASKPIECE pb)
//{
//	m_pTaskPiece=pb;
//}

void CBTStorage::SetConnectingRatio(float ratio)
{
	m_fConnectingRatio=ratio;

	unsigned int cons=(unsigned int)(m_nConnectingLinkMax*m_fConnectingRatio);

	m_PeerCenter.SetConnectingLimit(cons>100?100:cons);
}

void CBTStorage::SetConnectionRatio(float ratio)
{
	m_fConnectionRatio=ratio;
	m_PeerCenter.SetConnectionLimit((unsigned int)(m_nConnectionLinkMax*m_fConnectionRatio)); 

}

//read the priority in m_FilePriority and return int value
int CBTStorage::GetFilePriority(int seq)
{
	if( int(m_FilePriority.size()) <= seq) return 3; //default

	char cv=m_FilePriority[seq];

	//ÉÏ²ã±íÊŸÎÄŒþµÄÓÅÏÈŒ¶±ðÓÃ'0','1',....'F'ÀŽ±íÊŸ
	//'0'±íÊŸœûÖ¹ÏÂÔØ
	//'1'-'5'¶ÔÓŠÓÚ×îµÍ£¬µÍ£¬Õý³££¬žß£¬×îžß5žöÓÅÏÈŒ¶±ð
	//'6'-'A'¶ÔÓŠÓÚÉÏÃæµÄÔ€ÀÀÄ£Êœ£¬'B'-'F'±£Áô

	switch(cv)
	{
	case '0':
		return 0;
	case '1':
	case '6':
		return 1;
	case '2':
	case '7':
		return 2;
	case '3':
	case '8':
		return 3;
	case '4':
	case '9':
		return 4;
	case '5':
	case 'A':
	case 'a':
		return 5;
	default:
		return 3;
	}


}

bool CBTStorage::IsFileInPreviewMode(int seq)
{
	if(int(m_FilePriority.size()) <= seq) return false; //default

	char cv=m_FilePriority[seq];

	//ÉÏ²ã±íÊŸÎÄŒþµÄÓÅÏÈŒ¶±ðÓÃ'0','1',....'F'ÀŽ±íÊŸ
	//'0'±íÊŸœûÖ¹ÏÂÔØ
	//'1'-'5'¶ÔÓŠÓÚ×îµÍ£¬µÍ£¬Õý³££¬žß£¬×îžß5žöÓÅÏÈŒ¶±ð
	//'6'-'A'¶ÔÓŠÓÚÉÏÃæµÄÔ€ÀÀÄ£Êœ£¬'B'-'F'±£Áô

	switch(cv)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		return false;
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'a':
		return true;
	default:
		return false;
	}

}

//¶Ô¶àÎÄŒþÏÂÔØÀŽËµ£¬Èç¹ûÓÃ»§ÏÈÏÂÒ»²¿·Ö£¬¹Ø±Õ³ÌÐò£¬
//ÔÙÏÂÊ£Óà²¿·Ö£¬ÔòÎÒÃÇµÄÍê³ÉÆ¬ŒÇÂŒ»áÔÚ±ßÔµÊ§Ð§£¬³ÉÎª²»¿ÉÐÅµÄÆ¬
//ËùÒÔÃ¿ŽÎÆô¶¯ÈÎÎñ»òµ÷ÕûÎÄŒþÑ¡ÔñŽò¿ªÐÂÏÂÔØÎÄŒþÊ±£¬ÒªŒì²éœ»œÓµÄ
//±ßÔµŒÇÂŒÒÑÏÂÍêµÄÆ¬ÊÇ·ñ»¹ÓÐÐ§
//Œì²éµÄÌõŒþÊÇ£¬1£¬ÎÒÃÇŒÇÂŒÏÔÊŸÍê³ÉÁËÕâžöÆ¬£¬2£¬ÕâžöÆ¬ÊÇÎÄŒþ±ßÔµÆ¬£¬²»º¬ÁœÍ·
//3£¬ËùÓÐÒªÏÂÔØµÄÎÄŒþ¶ŒÒÑŽò¿ª£¬4£¬ÕâžöÆ¬Áœ±ßµÄÎÄŒþ¶ŒÊÇÑ¡ÔñÏÂÔØ×ŽÌ¬£¬5£¬Õâžö
//Æ¬ÊÇÁ¬œÓÁœžöÎÄŒþµÄÆ¬£¬»òÊÇ¿çÔœ¶àžöÎÄŒþµÄÆ¬

//²âÊÔ·¢ÏÖÕâžöº¯Êý¿ÉÄÜŽæÔÚÎóÅÐµÄ¿ÉÄÜ
bool CBTStorage::CheckEdgeIntegrality()
{

	int count=GetFinishedPieceCount();
	if(count==0) return false;


	//ÕÒµœËùÓÐÎÄŒþµÄœ»œçµãÎ»
	std::list<llong> intersect;

    llong globalpos=0;

	int num=m_pTorrentFile->GetFileNumber(true);

	if(num<=1) return false;

	for(int i=0;i<num;i++)
	{
		llong flen=m_pTorrentFile->GetFileLength(i,true);
		globalpos+=flen;
		intersect.push_back(globalpos);
	}


	//×îºóÒ»žöÊýŸÝÊÇÄ©Î²£¬ÉŸ³ýÖ®
	//intersect.pop_back();

	bool bitsetchanged=false;

	int blen=m_pTorrentFile->GetPieceLength();

	std::list<llong>::const_iterator it;
	for(it=intersect.begin();it!=intersect.end();it++)
	{
		//ºöÂÔÍêÈ«¶ÔÆë±ßœçµÄµãÎ»
		if( ((*it)%blen) ==0 ) continue;

		//²»¶ÔÆë£¬ÄÇÃŽÕâžöÎ»ÖÃ¶ÔÓŠÄÄžö¿é£¿
		unsigned int index= (unsigned int)((*it)/blen);

		//ÕâÀïžÄÎªËùÓÐµÄ±ßœç¶ŒÓÉŒì²éÊýŸÝÖØÉè±êŒÇ£¬ÒòÎªÓÐÊ±»áÒÆ×ßÒ»žöÎÄŒþÔÙÒÆ»ØÀŽ
		//ÎÒÃÇÊÇ·ñŒÇÂŒÁËÕâžö¿éÎªÓÐÐ§£¿
		//if(!IsFinishedPiece(index)) continue; 

		//¶ÁÈ¡ÊýŸÝŒì²éÕâžö±ßœç¿é
		std::string data;
			
		//if(ReadPiece(data,index)) 
		if(ReadPieceWithoutBuffer(data,index))
		{
			if(Tools::SHA1String(data)==m_pTorrentFile->GetPieceHash( index )) 
			{
				if(!IsFinishedPiece(index))
				{
					m_FinishedBitSet.Set(index,true);
					bitsetchanged=true;
				}
			}
			else
			{
				if(IsFinishedPiece(index))
				{
					bitsetchanged=true;
					m_FinishedBitSet.Set(index,false); 
				}
			}
		}
		else
		{

			if(IsFinishedPiece(index))
			{
				bitsetchanged=true;
				m_FinishedBitSet.Set(index,false);
			}
		}

	}

	return bitsetchanged;
}


//Èç¹û²»ÄÜœÓÊÜ£¬·µ»ØŒÙ£¬»¹ÊÇÓÉÔ­ŒàÌý·œÀŽ¹Ø±ÕÁ¬œÓ»ØÊÕ×ÊÔŽ
//·µ»ØÕæ£¬ÔòÕâžöÁ¬œÓÒÑŸ­±»±£ÁôœÓ¹Ü
bool CBTStorage::TransferPeer(CBTPeer* peer)
{

	//ÏÈ×öÒ»ŽÎ×ÜµÄŒì²é£¬±ÜÃâœÓÏÂÀŽµÄ¶àŽÎŒì²é
	if(!m_PeerCenter.CheckAccept(peer->GetPeeriIP()))
	{
		return false;
	}

	//³¢ÊÔŽ«ËÍžøÃ¿žösession£¬Èç¹û¶Œ²»ÄÜœÓÊÜ£¬·µ»Øfalse

    TSessionList::const_iterator it;
	SockLib::CAutoLock al(m_SessionListMutex);

    for ( it = m_SessionList.begin();it != m_SessionList.end();it++ )
    {
        if ( (*it) ->TransferPeer(peer) )
        {
			return true;
        }
    }


	return false;
}

void CBTStorage::SetSingleListener(CBTListener* listener)
{
	m_pSingleListener=listener;
}



std::string CBTStorage::GetPieceHash(int index)
{
	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());

	return m_pTorrentFile->GetPieceHash(index);
}

unsigned int CBTStorage::GetPieceCount()
{
	return m_pTorrentFile->GetPieceCount();
}


//»ºŽæÊÇÎÞÐòµÄ£¬ÏÈœøÏÈ³ö£¬¶¥³öÎ²œø
//Èç¹ûÃüÖÐÊýŸÝ£¬Ôò°ÑÊýŸÝÒÆ¶¯µœÎ²²¿
bool CBTStorage::ReadDataFromCache(std::string& data ,int index, unsigned int offset, unsigned int len)
{

	assert(index>=0);
	assert(index < m_pTorrentFile->GetPieceCount());

	{
		TReadBufferList::iterator it;

		SockLib::CAutoLock al(m_ReadCacheMutex);

		for(it=m_ReadBufferList.begin();it!=m_ReadBufferList.end();it++)
		{
			if(it->pIndex==index)
			{
				//Í¬Æ¬£¬ÊÇ·ñÓÐ¶ÔÓŠÊýŸÝ£¿ÊýŸÝ¿é³€¶ÈÓÐ¿ÉÄÜ²»ÊÇBUFFER_BLOCK_SIZE
				if( (it->offset <= offset) && (it->offset+it->data.size() >= offset + len))
				{

					TBufferBlock swap=(*it);
					m_ReadBufferList.erase(it); //œ»»»µœÎ²²¿
					data=swap.data.substr(offset-swap.offset,len); //ÕâÀïÔøŸ­³öŽíÁË
					m_ReadBufferList.push_back(swap); //œ»»»µœÎ²²¿
					return true;
				}

			}
		}
	}

//find in edge cache.

	if(IsEdgePiece(index))
	{
		SockLib::CAutoLock al(m_EdgeCacheMutex);

		TPieceMap::const_iterator it;

		it=m_EdgeCache.find(index);

		if(it!=m_EdgeCache.end())
		{
			data=it->second.substr(offset,len);
			return true;
		}
	}


	//ŽÓÐŽ»ºŽæÖÐÔÙ³¢ÊÔ¶Á
	{
		SockLib::CAutoLock al(m_WriteCacheMutex);

		TPieceMap::const_iterator it;

		it=m_WriteCache.find(index);

		if(it==m_WriteCache.end())	return false;

		data=it->second.substr(offset,len);
		return true;
	}

	return false;
}

bool CBTStorage::IsAvialbilityChanged()
{
	return false;
}

const char* CBTStorage::GetMyID(unsigned int sessionid)
{

#ifdef _SINGLEID
	return m_MyId;
#else
	return m_MyId+20*sessionid; //·µ»Ø¶ÔÓŠID
#endif
}

//ÎªÖÐÑëŒ¯ÖÐ¹ÜÀíPEERÊýŸÝµÄœÓ¿Ú
bool CBTStorage::GetPeerInfoToLink( unsigned int sessid, bool connectable,unsigned int& iip, unsigned short& iport, int& encref, unsigned int& timeout)
{
	return m_PeerCenter.GetPeerInfoToLink(sessid,connectable,iip,iport,encref,timeout);
}
//ÎªÖÐÑëŒ¯ÖÐ¹ÜÀíPEERÊýŸÝµÄœÓ¿Ú
bool CBTStorage::TryAcceptPeerLink( unsigned int sessid, unsigned int iip)
{
	return m_PeerCenter.TryAcceptPeerLink(sessid,iip);
}
//ÎªÖÐÑëŒ¯ÖÐ¹ÜÀíPEERÊýŸÝµÄœÓ¿Ú
void CBTStorage::LinkReport( unsigned int sessid, unsigned int iip, bool ok)
{
	m_PeerCenter.LinkReport(sessid,iip,ok);
}
//ÎªÖÐÑëŒ¯ÖÐ¹ÜÀíPEERÊýŸÝµÄœÓ¿Ú
void CBTStorage::CloseReport( unsigned int sessid, unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset)
{
	m_PeerCenter.CloseReport(sessid,iip,reason, accepted,peerid,bitset);
}

void CBTStorage::GiveUpLink( unsigned int sessid, unsigned int iip)
{
	m_PeerCenter.GiveUpLink(sessid,iip);
}

void CBTStorage::GetLinkStatus(unsigned int& mylink, unsigned int& peerlink, unsigned int& trylink, unsigned int& total)
{
	mylink=m_PeerCenter.GetMyInitConnected();
	peerlink=m_PeerCenter.GetPeerInitConnected();
	trylink=m_PeerCenter.GetConnecting();
	total=m_PeerCenter.GetTotalPeer();
}

//»ñµÃÐéÎÄŒþ¶ÔÕâžöÆ¬µÄÓ°ÏìÆðµãºÍ³€¶È£¬Õâ²¿·ÖÈ«Îª0
//·µ»ØŒÙÔò±íÊŸÃ»ÓÐÐéÎÄŒþµÄÓ°Ïì¡£
bool CBTStorage::GetAffectRangeByVirtualFileInPiece( int index, unsigned int& offset, unsigned int& len)
{

	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	//m_PieceInVirtualFile ÊÇ³£Êý£¬µÚÒ»ŽÎœšÁ¢ŸÍ²»»ážÄ±ä£¬²»ŒÓËøÊ¹ÓÃÁË¡£
	
	if(m_PieceInVirtualFile.empty()) return false; //Ã»ÓÐºÍÐéÄâÎÄŒþ¹ØÁªµÄÆ¬£¬ºöÂÔ

	TVirtualMap::const_iterator it;

	it=m_PieceInVirtualFile.find(index);

	if(it==m_PieceInVirtualFile.end()) return false;

	offset=it->second.offset;

	len=it->second.length;

	return true;
}
//not tested
void CBTStorage::NewBenliudPeer(unsigned int iip, unsigned short iport)
{
	m_PeerCenter.AddAvialableBenliudPeerInfo(iip,iport);
}



int CBTStorage::CheckBitSet( unsigned int sessid, std::string& peerid, unsigned int iip, CBTPiece& bitset)
{
	return m_PeerCenter.CheckBitSet(sessid, peerid, iip,bitset);
}

std::string CBTStorage::GenMyID( int seq )
{
/*
	//²âÊÔÎ±×°ID
	char idbuf[20];
#ifdef WIN32
	memcpy(idbuf,"-UT1750",7);  //œ«ÀŽŒÓÃÜ
#else
	memcpy(idbuf,"-KT2513",7);  //œ«ÀŽŒÓÃÜ
#endif

	//Ëæ»úÊýÓÉSHA1Éú³É

	std::string shash;

	unsigned int itime=GetTickCount();

	shash.append((const char*)idbuf,6);
	shash.append((const char*)(&itime),4);
	shash.append((const char*)(&seq),4);
	shash.append(m_pTorrentFile->GetInfoHash(),20);

	std::string rhash=SHA1String(shash);

	memcpy(idbuf+7, rhash.data(), 13);

	std::string idstr;
	idstr.append(idbuf,20);

	return idstr;


*/

	char idbuf[20];

#ifdef WIN32
	memcpy(idbuf,"M00250",6);  //œ«ÀŽŒÓÃÜ
#else
	memcpy(idbuf,"M00251",6);  //œ«ÀŽŒÓÃÜ
#endif

	//Ëæ»úÊýÓÉSHA1Éú³É

	std::string shash;

	unsigned int itime=GetTickCount();

	shash.append((const char*)idbuf,6);
	shash.append((const char*)(&itime),4);
	shash.append((const char*)(&seq),4);
	shash.append(m_pTorrentFile->GetInfoHash());

	std::string rhash=Tools::SHA1String(shash);

	memcpy(idbuf+6, rhash.data(), 14);

	//ŽŠÀí17,18Î»
	idbuf[17]=idbuf[17]&0xFD;	//0xFD=11111101b
	idbuf[17]=idbuf[17]|0x20;	//0x20=00100000b

	idbuf[18]=idbuf[18]&0xFB;	//0xFB=11111011b
	idbuf[18]=idbuf[18]|0x40;	//0x40=01000000b

	//ŽŠÀíµÚ16Î»
	int i;

	char sum=idbuf[6];
	for(i=7;i<=15;i++)
	{
		sum^=idbuf[i];
	}

	idbuf[16]=sum;

	//ŽŠÀíÇ°6Î»µÄŒÓÃÜ

	idbuf[0]^=idbuf[17];
	idbuf[1]^=idbuf[18];
	idbuf[2]^=idbuf[17];
	idbuf[3]^=idbuf[18];
	idbuf[4]^=idbuf[17];
	idbuf[5]^=idbuf[18];

	//ŽŠÀíµÚ19Î»
	sum=idbuf[0];
	for(i=1;i<19;i++)
	{
		sum^=idbuf[i];
	}

	idbuf[19]=sum;

	std::string idstr;
	idstr.append(idbuf,20);

	return idstr;
	
}

void CBTStorage::LinkOkButNoRoomClose( unsigned int sessid, unsigned int iip)
{
	m_PeerCenter.LinkOkButNoRoomClose(sessid,iip);
}

void CBTStorage::LinkOkButPeerClose(unsigned int sessid, unsigned int iip)
{
	m_PeerCenter.LinkOkButPeerClose(sessid,iip);
}

void CBTStorage::GiveUpAcceptPeerLink( unsigned int sessid, unsigned int iip)
{
	m_PeerCenter.GiveUpAcceptPeerLink(sessid, iip);
}

llong CBTStorage::GetFinishedBytes()
{
	llong nret=0;

	unsigned int allsize = m_FinishedBitSet.GetSize();

	for ( unsigned int i = 0;i < allsize;i++ )
	{

		if ( m_FinishedBitSet.IsSet( i ) )
		{
			nret+=GetPieceLength(i);
		}

	}

	return nret;
}

llong CBTStorage::GetUnFinishedBytes()
{
	llong nret=0;

	unsigned int allsize = m_FinishedBitSet.GetSize();

	for ( unsigned int i = 0;i < allsize;i++ )
	{

		if ( !m_FinishedBitSet.IsSet( i ) )
		{
			nret+=GetPieceLength(i);
		}

	}

	return nret;
}

//check ok
void CBTStorage::CleanOrphan(int index)
{
	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());

	SockLib::CAutoLock al(m_OrphanMapMutex);

	m_OrphanMap.erase(index);
}

void CBTStorage::CheckOutOrphanData(int index, std::list<COrphan>& orphans)
{
	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());

	SockLib::CAutoLock al(m_OrphanMapMutex);

	TOrphanMap::iterator it;

	it=m_OrphanMap.find(index);

	if(it==m_OrphanMap.end()) return ; //no orphan

	//have orphans
	orphans.swap(it->second);

	m_OrphanMap.erase(index);

	return ;
}

void CBTStorage::CheckInOrphanData(int index, std::list<COrphan>& orphans)
{

	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	SockLib::CAutoLock al(m_OrphanMapMutex);

	TOrphanMap::iterator it;

	it=m_OrphanMap.find(index);

	if(it==m_OrphanMap.end()) 
	{
		m_OrphanMap[index]=orphans;
	}
	else
	{
		std::list<COrphan>::const_iterator it2;
		for(it2=orphans.begin();it2!=orphans.end();it2++)
		{
			it->second.push_back((*it2));
		}
	}
}

//for single orphan
void CBTStorage::CheckInOrphanData(int index, unsigned int iip, unsigned int offset, std::string& data)
{
	assert(index >= 0);
	assert(index < m_pTorrentFile->GetPieceCount());

	SockLib::CAutoLock al(m_OrphanMapMutex);

	TOrphanMap::iterator it;

	it=m_OrphanMap.find(index);

	if(it==m_OrphanMap.end()) 
	{
		std::list<COrphan> empty;
		COrphan item;
		item.data=data;
		item.index=index;
		item.offset=offset;
		item.source=iip;
		empty.push_back(item);
		m_OrphanMap[index]=empty;
	}
	else
	{
		COrphan item;
		item.data=data;
		item.index=index;
		item.offset=offset;
		item.source=iip;
		m_OrphanMap[index].push_back(item);
	}
}


//Íš¹ý·ÖÎöfinished bitsetÌî³äTStorageFileInfoÖÐµÄ
//comppiece žÃÎÄŒþÍê³ÉÆ¬Êý, lastwrite žÃÎÄŒþ×îºóÐŽÈëµÄÎ»ÖÃ
//Õâžöº¯ÊýÖ»ÔÚ³õÊŒ»°ÈÎÎñÊ±µ÷ÓÃ£¬ÓÃÓÚ³õÊŒ»¯œá¹¹
void CBTStorage::InitFileProgress()
{
	TFileHandleList::iterator it;

	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile||it->fileInfo.size==0)
		{
			it->comppiece=0;
			it->lastwrite=it->headindex;
			continue;
		}

		it->comppiece=0;
		it->lastwrite=it->headindex;

		for(int i=it->headindex; i< it->tailindex;i++)
		{
			if(m_FinishedBitSet.IsSet(i))
			{
				it->comppiece++;
				it->lastwrite=i;
			}
		}

	}
}

//ÍšÖªÎÄŒþœø¶ÈµœœçÃæ£¬·Ç»î¶¯ÈÎÎñ²»ÐèÒªÍšÖª
void CBTStorage::NoticeFileProgress()
{
	int count=0;	//³ýÈ¥ÐéÎÄŒþµÄÎÄŒþÐòºÅ

	TFileHandleList::iterator it;

	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile)
		{
			continue;
		}
		else
		{

			float percent=1.0;

			if(it->tailindex != it->headindex)
			{
				percent= float(it->comppiece)/float(it->tailindex - it->headindex);
			}

			//TOFIX
			//m_pTaskEvent( m_nTaskId, _FILE_PERCENT, llong(count), llong(percent*10000),NULL); //ÍšÖªÎÄŒþÍê³É±ÈÀý

			count++;
		}

	}
}

void CBTStorage::RecordNewPieceInFile(int index)
{
	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	TFileHandleList::iterator it;

	int count=0; //³ýÈ¥ÐéÎÄŒþµÄÎÄŒþÐòºÅ

	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile)
		{
			continue;
		}

		if(it->headindex <= index && index < it->tailindex)
		{
			it->comppiece++;
			//if(it->lastwrite < index) it->lastwrite=index;

			//if(m_bActive)
			//{

				float percent=1.0;
				if(it->tailindex != it->headindex)
				{
					percent= float(it->comppiece)/float(it->tailindex - it->headindex);
				}

				//TOFIX ,把文件进度送到BTJob内
				//m_pTaskEvent( m_nTaskId, _FILE_PERCENT, count, llong(percent*10000),NULL); //ÍšÖªÎÄŒþÍê³É±ÈÀý
			//}
		}
		else if( index < it->headindex )
		{
			break;
		}

		count++;
	}
}



//ÖÇÄÜÐŽÅÌŽŠÀí£¬±ÜÃâ³€Ê±ŒäÐŽÅÌ
//ŽíÎó·µ»Ø<0, ·ñÔò·µ»ØÐŽÈëµÄÊýŸÝÆ¬
int CBTStorage::SmartWriteCache()
{
	unsigned int start=GetTickCount();

	int wrote=0; //±ŸŽÎÐŽÈëµÄÆ¬Êý

	SockLib::CAutoLock al(m_WriteCacheMutex);


	TPieceMap::iterator it;
			
	for ( it = m_WriteCache.begin(); it != m_WriteCache.end();)
	{
		if(PieceDistanceFromLastWrite(it->first) <= 4*1024*1024)
		{
			int nret=WritePieceToDisk( it->first, it->second );
			if(nret!=0) return nret; //some write error happens
			
			wrote++;
			
			RecordLastWriteInFile(it->first);

			m_WriteCache.erase(it++);

			if(GetTickCount() - start > 100) break; //Ê±Œä¹ýÁË£¬²»ÐŽÁË
		}
		else
		{
			it++; 
			continue;
		}
	}


	unsigned int maxsize= 15*1024*1024 / m_pTorrentFile->GetPieceLength();

	if( maxsize <= m_WriteCache.size())
	{
		int nwrite= int(m_WriteCache.size() - maxsize); 

	
		for ( it = m_WriteCache.begin(); nwrite>=0 && it != m_WriteCache.end();)
		{
			int nret=WritePieceToDisk( it->first, it->second );
			if(nret!=0) return nret;

			wrote++;

			RecordLastWriteInFile(it->first);
			m_WriteCache.erase(it++);
			nwrite--;
		}

	}

	if(GetTickCount() - start > 100) 
	{
		return wrote;	//ÐŽÈëÊ±ŒäÒÑŸ­Ì«Žó£¬·µ»Ø
	}

	//ÓÐž»Ô£Ê±ŒäµÄ»°£¬¶ÔŽóÎÄŒþÂýÂýÐŽÈëÒ»Ð©0ÀŽÍÆœøÎÄŒþÄ©Î²µÄÎ»ÖÃ
	//Ÿ­¹ýÊµÑé£¬²»ÄÜÒÔÐŽÈë0»ò¶ÁÈ¡ÎÄŒþÀŽÑÓ³€ÎÄŒþÄ©Î²£¬ÎÒÃÇÐŽÈë1žö×ÖœÚµÄ0x0AŒŽ¿É
	//Ò»ŽÎÐŽÅÌ¿çŸà4MÀŽÍÆœøÎÄŒþÄ©Î²¡£¶ÔÓÚÐ¡ÓÚ4MµÄÎÄŒþ²»ÐèÒª×öÕâžö²Ù×÷

	int piecestep= 4*1024*1024 / m_pTorrentFile->GetPieceLength(); //²œ³€Îª4M±ÈœÏºÏÊÊ

	if(piecestep==0) piecestep=1; //¿ÉÄÜ¿éÒÑŸ­³¬¹ý4MÁË, ²œ³€×ª»»Îª¿éµÄ²œ³€

	TFileHandleList::iterator it2;

	for(it2=m_FileHandleList.begin();it2!=m_FileHandleList.end();it2++)
	{
#if defined(WINCE)||defined(WIN32)
		if(it2->fileInfo.vfile||it2->handle==INVALID_HANDLE_VALUE||it2->fileInfo.size<=4*1024*10240 /*|| it2->lastwrite + piecestep >= it2->tailindex*/ )
#else
		if(it2->fileInfo.vfile||it2->handle==-1||it2->fileInfo.size<=4*1024*10240 /*|| it2->lastwrite + piecestep >= it2->tailindex*/ )
#endif
		{
			continue;
		}

		/*unsigned*/ int piecepos=it2->lastwrite + piecestep;

		if(piecepos >= it2->tailindex) 
		{
			it2->lastwrite=it2->tailindex; 
			continue;
		}


		llong goffset= llong(piecepos)* llong(m_pTorrentFile->GetPieceLength());

		//
		if(goffset >= it2->fileInfo.offset + it2->fileInfo.size) continue; 

		//È«ŸÖÆ«Î»ŒõÒ»
		goffset-=1; 

		//ÎÄŒþÄÚÆ«Î»
		llong foffset= goffset- it2->fileInfo.offset;


		char ec=0x0A; //²âÊÔÏÔÊŸÐŽÈë0ÊÇÎÞÐ§µÄ£¬»á±»ÏµÍ³ºöÂÔ

#if defined(WINCE)||defined(WIN32)
		LARGE_INTEGER li;
		li.QuadPart=foffset;
		DWORD dwRet=::SetFilePointer(it2->handle, li.LowPart, &li.HighPart, FILE_BEGIN); 
		if(dwRet==0xFFFFFFFF) return -10;
#else
        llong check1=lseek64( it2->handle, foffset, SEEK_SET ); //ÒÆ¶¯µœÎÄŒþÄÚÆ«ÒÆ
		if(check1!=foffset) return -10;
#endif
		

#if defined(WINCE)||defined(WIN32)
		DWORD dwWrote;
		::WriteFile(it2->handle, &ec, 1, &dwWrote, NULL);
#else
		long check2=write(it2->handle,&ec,1);
		if(check2!=1) return -11;
#endif	
	
		it2->lastwrite+=piecestep;

		if(GetTickCount() - start > 100) break;	//ÐŽÈëÊ±ŒäÒÑŸ­Ì«Žó£¬·µ»Ø
		else continue;

	}

	return wrote;
}

//ŒÇÂŒÎÄŒþ×îºóÐŽµÄÎ»ÖÃ£¬
void CBTStorage::RecordLastWriteInFile(int index)
{

	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());

	TFileHandleList::iterator it;

	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile)
		{
			continue;
		}

		if(it->headindex <= index && index < it->tailindex)
		{
			if(it->lastwrite < index) it->lastwrite=index;
			break;
		}

	}
}



llong CBTStorage::PieceDistanceFromLastWrite(int index)
{
	assert(index>=0);
	assert(index<m_pTorrentFile->GetPieceCount());


	TFileHandleList::iterator it;

	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->fileInfo.vfile||it->fileInfo.size==0)
		{
			continue;
		}

		if( it->headindex <=index && index < it->tailindex )
		{
			if((index - it->lastwrite - 1) <=0) return 0;

			return llong( index - it->lastwrite - 1 ) * llong(m_pTorrentFile->GetPieceLength());
		}
	}

	return 0;
}

//check if this piece is edge piece between two files
//all edge piece data store in memory
bool CBTStorage::IsEdgePiece(int index)
{
	TFileHandleList::const_iterator it;
	for(it=m_FileHandleList.begin();it!=m_FileHandleList.end();it++)
	{
		if(it->headindex==index) return true;
	}
	return false;
}



bool CBTStorage::AnyUnCheckedNode()
{
	return m_PeerCenter.AnyUnCheckedNode();
}

//Á¬œÓÍš¹ýÕâžöº¯Êý±šžæ¶Ô·œÊÇ·ñÖ§³ÖŒÓÃÜ
void CBTStorage::PeerSupportEncryption(unsigned int iip, bool enc)
{
	m_PeerCenter.PeerSupportEncryption(iip,enc);
}



int CBTStorage::GetPeerCredit(unsigned int iip)
{
	return m_PeerCenter.GetPeerCredit(iip);
}

void CBTStorage::MakeSureDirectoryExistsForFile(wchar_t* filePath)
{
	// \storage\folder1\folder2\file.txt
	//build \storage and then \storage\folder1 and then \storage\folder1\folder2

	wchar_t* pMove;
	
	wchar_t tempdir[MAX_PATH];


	bool found=false;

	for(int want=2; ;want++)
	{
		int count=0;
		wcscpy(tempdir, filePath);
		pMove=tempdir;

		found=false;

		while(*pMove)
		{
			if(*pMove==L'\\')
			{
				count++;
				if(count==want)
				{
					*pMove=L'\0';
					::CreateDirectory(tempdir, NULL);
					found=true;
					break;
				}
			}
			
			pMove++;

		}//while

		if(!found) break;
	}

}

_JOB_STATUS CBTStorage::GetJobStatus()
{
	return m_JobStatus;
}
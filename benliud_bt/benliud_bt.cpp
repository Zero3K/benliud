/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


/* Created by Anjuta version 1.2.4a */


#ifdef WIN32
#pragma warning (disable: 4786)
#endif


#include "../benliud/module_def.h"
#include "../benliud/protocol_def.h"
#include <Tools.h>
//#include "../benliud/callback_def.h"
#include "../benliud/bittorrent_types.h"
#include <Mutex.h>
#include "./include/BTJob.h"
#include "./include/BTListener.h"
#include <assert.h>

//
//struct _NewJobStruct
//{
//	int jobid;
//	int torsize; //torrent data size
//	const char* ptorrent;
//	int bitsize; //bitset data size
//	const char* pbitset; 
//	int prisize; //priority size
//	const char* priority;
//	const wchar_t* savefolder;	//where to save data file
//	const wchar_t* encode;	//encode string.
//	int uplimit;	//upload speed limit in kb/s
//	int dwlimit;	//download speed limit in kb/s
//	int maxconn;	
//	int cache;		//in kb
//	_BT_STOPMODE stopmode; //stop mode	
//	//auto cache size
//	//auto connection limit
//};

/*
 * all module should have these five function,
 * they are modtype(),moduuid(),modvers(),moddesc(), modauth() and setsyslog2()
 */

#include <vector>

#if defined(WIN32)||defined(__WXMSW__)||defined(WINCE)
#include <winsock2.h>
#include <windows.h> 

#define DllExport   extern "C" __declspec( dllexport )

BOOL APIENTRY DllMain( HANDLE hModule,
                        DWORD ul_reason_for_call,
                        LPVOID lpReserved
                      )
{

    return TRUE;
}

#else
#define DllExport
#endif

class TJobItem
{
public:
    int taskid;
    int jobid;
    CBTJob* pjob;
};

static SockLib::CMutex jobMutex;

static CBTListener *listener=NULL;


//global argument
static unsigned int global_con_limit=100;	//连接数限制，应不少于200

//连接发起限制
static unsigned int global_try_limit=30;	//应不少于10


//global upload speed limit
static int	global_upload_speed=0;
//global download speed limit
static int  global_download_speed=0;

static int jobcounter = 0;

static std::vector<TJobItem> btJOB;


void syslog( std::string info )
{
    FILE * fp;
    fp = fopen( "\\benliud.txt", "a+" );
    if ( fp == NULL )
        return ;

    fprintf( fp, "%s", info.c_str() );

    fclose( fp );
}


//WINCE need this 
DWORD GetAvailMemory()
{

	MEMORYSTATUS memstatus;
	memstatus.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&memstatus);
	return memstatus.dwAvailPhys;
	
}

void logsysmem(int id)
{
	DWORD mem=GetAvailMemory();
	char buf[30];
	sprintf(buf, "%d: %d\n", id, mem);
	syslog(buf);
}


DllExport 
_MODULE_TYPE modtype()
{
    return _MOD_PROT;  //this module is a protocol module
}

DllExport 
wchar_t*	moduuid()
{
	return L"ee7334a0-edc5-11dc-95ff-0800200c9a66";
}


DllExport 
wchar_t*	modvers()
{
	return L"0.3.3.0 for PPC [2010/02/15]";
}

DllExport 
wchar_t* moddesc()
{
	return  L"Benliud's bittorrent module.";
}

DllExport 
wchar_t* modauth()
{
	return L"liubin,China";
}


/* protocol module should have those functions  */
/* return protocol type  */
DllExport 
_PROT_TYPE	protype()
{
    return _PROT_BITTORRENT;	//it's a http protocol provider.
}

/* create a job instance, return the job id.  */
DllExport 
int createjob( int taskid )
{

	jobMutex.Lock();
	
    TJobItem newjob;
    newjob.taskid = taskid;
    newjob.jobid = ++jobcounter;
    newjob.pjob = NULL;
    btJOB.push_back( newjob );

	jobMutex.Unlock();
    return newjob.jobid;

}


//delete task just clean the item in list
DllExport 
void deletejob( int taskid )
{

	
	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == taskid )
        { //ok ,found the item
            //because pjob is a detached thread, it will delete itself when quit,
            //so we don't need delete the pjob, just clean it in list
            
			if(it->pjob!=NULL)	{
				it->pjob->Stop();
				delete it->pjob;
			}

            btJOB.erase( it );

            break;
        }
    }

	
	jobMutex.Unlock();

    return ;

}

/* how many job is created  */
DllExport int getjobnum()
{
    return btJOB.size();
}




DllExport
bool beginjob(_NewJobStruct* pData)
{

    bool result = false;


	jobMutex.Lock();


    //try to find job in list 
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin(); it != btJOB.end(); it++ )
    {
        if ( it->jobid == pData->jobid )
        { //ok ,found the item
            //if job still running,delete job will have error.
            if ( it->pjob != NULL )
                delete it->pjob;


            it->pjob = new CBTJob(
							it->taskid,
							pData->torsize,
							pData->ptorrent,
							pData->bitsize,
							pData->pbitset,
							pData->savefolder,
							pData->codepage,
							pData->uplimit,
							pData->dwlimit,
							global_try_limit,
							pData->maxconn, //maxconn
							pData->cache, //cache
							_PREFER_ENCRYPT,//_PREFER_ORDINARY, _ALWAYS_ORDINARY, _PREFER_ENCRYPT// //encmode
							pData->stopmode,
							pData->prisize,
							pData->priority
                       );


			it->pjob->SetSingleListener(listener);


            if ( it->pjob->Go() )
            {
                result = true;
            }
            else
            { //clean object
                delete it->pjob;
                btJOB.erase( it );
            }

            break;
        }
    }


	jobMutex.Unlock();

    return result;
}

//just set the stop mark ,when the task stopped, gui will got the
//event and delete the job object
DllExport 
void stopjob( int taskid )
{

	
	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == taskid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->Stop();
            }
            else
            {
                //must have some wrong things
                //the job not begin, and then stop?
                //should just call deletejob to remove the job in list
				assert(false);
            }

            break;
        }
    }

	
	jobMutex.Unlock();

}

//adjust priority interface
//the data is a group ,data[2*i+0] is fileseq, data[2*i+1] is priority
//count is the number of data group
DllExport 
void adjpriori( int taskid, const char* prios )
{

	
	jobMutex.Lock();


    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == taskid )
        {
            if ( it->pjob != NULL )
            {

                it->pjob->AdjustFilePriority(prios);
            }
            break;
        }
    }

	jobMutex.Unlock();
}




DllExport
void adjustupspeed(int tid, int speed) //adjust upload speed 
{

	
	jobMutex.Lock();


    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustUpSpeed(speed);
            }
            break;
        }
    }


	
	jobMutex.Unlock();
}

DllExport
void adjustdwspeed(int tid, int speed) //adjust download speed 
{
	//try to find the task in list and set it
	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustDownSpeed(speed);
            }
            break;
        }
    }
	
	jobMutex.Unlock();
}

DllExport
void adjustencrypt(int tid, _BT_ENCMODE mode) //adjust ecnryption mode
{
	//try to find the task in list and set it
	jobMutex.Lock();


    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustEncryptMode(mode);
            }
            break;
        }
    }


	jobMutex.Unlock();
}

DllExport
void adjustmaxlink(int tid, int link) //adjust max link
{

	
	jobMutex.Lock();


    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustMaxConnection(link);
            }
            break;
        }
    }


	
	jobMutex.Unlock();
}

DllExport
void adjustcachesize(int tid, int size)
{

	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustCacheSize(size);
            }
            break;
        }
    }


	jobMutex.Unlock();
}

DllExport
void adjuststopmode(int tid, _BT_STOPMODE mode)
{

	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AdjustStopMode(mode);
            }
            break;
        }
    }


	jobMutex.Unlock();
}

//多任务时平衡连接数和连接发起数
DllExport
void balence()
{

	if(btJOB.empty()) return;

	//两个任务降低为1个时也需要平衡。

	//if(btJOB.size() <=1) return; //no balence for single task or no task
    std::vector<TJobItem>::iterator it;


	int notfinishconsum=0;	//连接总数
	int finishjobconsum=0;	//连接总数

	
	int notfinishjob=0;
	int finishedjob=0;

	jobMutex.Lock();

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->pjob != NULL )
        {
            if(it->pjob->IsFinished())
			{
				finishjobconsum+=it->pjob->GetMaxConnection();
				finishedjob++;
			}
			else
			{
				notfinishconsum+=it->pjob->GetMaxConnection();
				notfinishjob++;
			}
        }

    }

	//check connecting limit
	if(notfinishjob==0 && finishedjob==0) 
	{
		jobMutex.Unlock();
		return;
	}
	else if(notfinishjob == 0)
	{//all job is uploading, just reduce the try equally
		float ratio= 1.0f / float(finishedjob);
		
		for ( it = btJOB.begin();it != btJOB.end();it++ )
		{
			if(it->pjob != NULL)
			{
				it->pjob->SetConnectingRatio( ratio );
			}
		}
		
	}
	else if(finishedjob==0)
	{//all job not finish
		float ratio= 1.0f / float(notfinishjob);
		
		for ( it = btJOB.begin();it != btJOB.end();it++ )
		{
			if(it->pjob != NULL)
			{
				it->pjob->SetConnectingRatio( ratio );
			}
		}		
	}
	else
	{//have finished and not finished job
		
		float ratio1;
		float ratio2;
		
		ratio1= (0.9f)/float(notfinishjob); //ratio for not finished job
		ratio2= (0.1f)/float(finishedjob); //ratio for finished job
		
		//ratio1+ratio2==1.0
		
		for ( it = btJOB.begin();it != btJOB.end();it++ )
		{
			if(it->pjob != NULL )
			{
				if(it->pjob->IsFinished())
					it->pjob->SetConnectingRatio(ratio2);
				else 
					it->pjob->SetConnectingRatio(ratio1);
			}
			
		}
		
	}

//TODO: check it from here

	if(global_con_limit < finishjobconsum + notfinishconsum)
	{//adjust
		if(notfinishjob == 0)
		{//all job is finished
			float ratio= float(global_con_limit)/float(finishjobconsum);

			for ( it = btJOB.begin();it != btJOB.end();it++ )
			{
				if(it->pjob != NULL)
				{
					it->pjob->SetConnectionRatio(ratio);
				}
			}
		}
		else if(finishedjob==0)
		{//all job not finish
			float ratio= float(global_con_limit)/float(notfinishconsum);

			for ( it = btJOB.begin();it != btJOB.end();it++ )
			{
				if(it->pjob != NULL)
				{
					it->pjob->SetConnectionRatio(ratio);
				}
			}
		}
		else
		{//we have not finished job,  the not finished job have 4/5 connection 
			float ratio1;
			float ratio2;

			ratio1= (0.8f)/float(notfinishjob); //ratio for not finished job
			ratio2= (0.2f)/float(finishedjob); //ratio for finished job

			for ( it = btJOB.begin();it != btJOB.end();it++ )
			{
				if(it->pjob != NULL )
				{
					if(it->pjob->IsFinished())
						it->pjob->SetConnectionRatio(ratio2);
					else 
						it->pjob->SetConnectionRatio(ratio1);
				}

			}
		}
	}

	
	jobMutex.Unlock();
}



DllExport
bool startservice(unsigned int port)
{
	//start the listener for single port listen
	if(listener!=NULL) return true;
	
	listener=new CBTListener;
	listener->SetDownloadSpeedLimit(global_download_speed);
	listener->SetUploadSpeedLimit(global_upload_speed);


	bool bret=listener->Start(port);

	if(!bret)
	{
		listener->Stop();
		delete listener;
		listener=NULL;
		return false;
	}

	return true;
}



DllExport
void stopalljob()
{
	jobMutex.Lock();

    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;
	
    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
		if ( it->pjob != NULL )
		{
			it->pjob->Stop();
			delete it->pjob;
			it->pjob=NULL;
        }
    }
	
	jobMutex.Unlock();

}

DllExport
void stopservice()
{
	//首先要停止所有的任务，否则监听没有了再停任务就会错
	//不过停止所有任务在这里处理不了，要由上层处理

	if(listener==NULL) return;

	stopalljob();

	listener->Stop();
	delete listener;
	listener=NULL;
}


DllExport
int getrunnum()
{
	jobMutex.Lock();

	int count=0;
    //try to find job in list and remove it
    std::vector<TJobItem>::iterator it;
	
    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
		if ( it->pjob != NULL )
		{
			count++;
        }
    }
	
	jobMutex.Unlock();

	return count;
}

DllExport
void setconlimit(int conn)
{
	global_con_limit=conn;
	balence();

}

DllExport
void settrylimit(int trys)
{
	global_try_limit=trys;

	if(global_try_limit>250) global_try_limit=250; //太大的发起连接并不好，没有必要

}

//global speed limit

DllExport
void setdwspeed(int speed)
{
	if(speed==0) global_download_speed=0x0FFFFFFF;
	else global_download_speed=speed*1024;

	if(listener!=NULL)
	{
		listener->SetDownloadSpeedLimit(global_download_speed);
	}

}

DllExport
void setupspeed(int speed)
{
	if(speed==0) global_upload_speed=0x0FFFFFFF;
	else global_upload_speed=speed*1024;

	if(listener!=NULL)
	{
		listener->SetUploadSpeedLimit(global_upload_speed);
	}

}

//外部提取信息接口

//获得任务状态
DllExport
bool getstatus(int tid, _JOB_STATUS* status, float* avail)
{

	bool bret=false;

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
			if ( it->pjob != NULL )
            {
                *status=it->pjob->GetStatus();
				*avail=it->pjob->GetAvailability();
				bret=true;
            }
            break;
        }
    }

	jobMutex.Unlock();

	return bret;
}

DllExport
bool getspeed(int tid, int* dwspd, int* upspd)
{

	bool set=false;

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                *dwspd=it->pjob->GetDownSpeed();
				*upspd=it->pjob->GetUpSpeed();
				set=true;
            }
            break;
        }
    }

	jobMutex.Unlock();

	return set;
}

//获得下载速度
DllExport
int getdwspd(int tid) //return in kb/s
{

	int spd=0;

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                spd=it->pjob->GetDownSpeed();
            }
            break;
        }
    }

	jobMutex.Unlock();

	return spd;
}

//获得上传速度
DllExport
int getupspd(int tid) //return in kb/s
{

	int spd=0;

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                spd=it->pjob->GetUpSpeed();
            }
            break;
        }
    }

	jobMutex.Unlock();

	return spd;
}

DllExport
float getprogress(int tid)
{
	float pgs=0.0f;

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                pgs=it->pjob->GetProgress();
            }
            break;
        }
    }

	jobMutex.Unlock();

	return pgs;
}

void addpeers(int tid, int nbyte, const char* pdata)
{

	jobMutex.Lock();

    std::vector<TJobItem>::iterator it;

    for ( it = btJOB.begin();it != btJOB.end();it++ )
    {
        if ( it->taskid == tid )
        {
            if ( it->pjob != NULL )
            {
                it->pjob->AddPeers(nbyte, pdata);
            }
            break;
        }
    }

	jobMutex.Unlock();

	return ;
}
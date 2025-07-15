
#ifndef _DATATYPE_DEF_H
#define _DATATYPE_DEF_H

#if defined( WIN32 )||defined (__WXMSW__)||defined(WINCE)
typedef __int64 llong;
typedef unsigned __int64 ullong;
#include <stdlib.h>
#include <stddef.h>
#else
typedef long long int llong ;
typedef unsigned long long int ullong ;
#endif


enum _TASKSTATUS
{
    _TASK_ERROR=0,	
    _TASK_RUNNING=1,	
    _TASK_WAITTING=2,
	_TASK_STOP=3,
	_TASK_STOPING=4,
	_TASK_DELETING=5,	//the task is running but we delete it
	_TASK_DELETING_WITHFILE=6, //deleting and with file delete
	_TASK_UPLOADING=7,	//纯粹的上传中，任务已经完成
};

enum _JOB_STATUS
{
	_JOB_NONE,
	_JOB_CHECKINGFILE,
	_JOB_RUNNING,
	_JOB_DOWNLOADING,
	_JOB_UPLOADING,
	_JOB_FAILED,
	_JOB_QUIT,
};

#endif

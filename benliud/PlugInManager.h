/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#pragma once

#include "bittorrent_types.h"
#include "datatype_def.h"
#include <Winnls.h> //codepage

typedef bool (*pfstartservice_bt)(unsigned short);
typedef void (*pfstopservice_bt)();
typedef void (*pfstopalljob_bt)();
typedef int  (*pfcreatejob_bt)( int );
typedef bool (*pfbeginjob_bt)(_NewJobStruct*);
typedef void (*pfdeletejob_bt)( int );
typedef void (*pfaddpeers_bt)(int tid, int nbyte, const char* pdata);
typedef float  (*pfgetprogress_bt)(int tid);
typedef bool (*pfgetspeed_bt)(int tid, int* dwspd, int* upspd);
typedef bool (*pfgetstatus_bt)(int tid, _JOB_STATUS* status, float* avail);

typedef bool (*pfstartservice_btkad)(unsigned short);
typedef void (*pfstopservice_btkad)();
typedef bool (*pfremovetask_btkad)(char*);
typedef void (*pfstopservice_btkad)();
typedef bool (*pfaddtask_btkad)(char* hash, unsigned short port);
typedef bool (*pfaddtask_btkad)( char* , unsigned short);
typedef int  (*pgetpeers_btkad)(char* , int , char* , int* );

typedef bool (*pfstartservice_upnp)();
typedef void (*pfstopservice_upnp)();
typedef UINT (*pfgetstate_upnp)();
typedef bool (*pfgetexternip_upnp)(char*);
typedef	void (*pfaddportmap_upnp)(unsigned short port,bool tcp);



class CPlugInManager
{
public:
	CPlugInManager(void);
	~CPlugInManager(void);
	bool Initial(CString dir);
	bool StartServices(unsigned short btport, unsigned short kadport, unsigned short reserved);
	void StopServices();
	
	void AddTaskToKad(char* phash);
	void DelTaskFromKad(char* phash);

	int  CreateTaskToBT(int taskid); //return job id
	bool AddTaskToBT(_NewJobStruct& task);
	void DelTaskFromBT(int taskid);
	void DelAllTaskFromBT();

	bool GetTaskStatus(int taskid, _JOB_STATUS* status, float* avial); //bt
//	void GetTaskSpeed(int taskid);	//bt

	int GetPeersFromKad(char* hash, int buflen, char* buf, int* total);
	void AddPeersToTask(int tid, int nbyte, const char* pdata);

	float GetProgress(int tid);
	bool GetSpeed(int tid, int& dwspd, int& upspd);
private:
	HINSTANCE m_hBTMod;
	HINSTANCE m_hUPNPMod;
	HINSTANCE m_hBTKADMod;
	unsigned short m_btport;
};

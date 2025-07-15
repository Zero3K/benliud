/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#include "StdAfx.h"
#include "PlugInManager.h"

CPlugInManager::CPlugInManager(void)
{
	m_hBTMod=NULL;
	m_hUPNPMod=NULL;
	m_hBTKADMod=NULL;
	m_btport=0;
}

CPlugInManager::~CPlugInManager(void)
{
}

bool CPlugInManager::Initial(CString dir)
{
	//dir is current folder

	//简化版本
	m_hBTKADMod=::LoadLibrary(L"benliud_btkad.dll");
	if(m_hBTKADMod==NULL) {
		::MessageBox(NULL, L"load bt kad failed.", L"fail", MB_OK);
		return false;
	}

	m_hUPNPMod=::LoadLibrary(L"benliud_upnp.dll");
	if(m_hUPNPMod==NULL) {
		::MessageBox(NULL, L"load bt upnp failed.", L"fail", MB_OK);
		return false;
	}

	m_hBTMod=::LoadLibrary(L"benliud_bt.dll");
	if(m_hBTMod==NULL) {
		::MessageBox(NULL, L"load bt mod failed.", L"fail", MB_OK);
		return false;
	}

	return true;
}

bool CPlugInManager::StartServices(unsigned short btport, unsigned short kadport, unsigned short reserved)
{
	//start upnp
	pfstartservice_upnp pstart1=(pfstartservice_upnp)GetProcAddress(m_hUPNPMod, L"startservice");
	if(pstart1==NULL || !pstart1()) return false;

	//start kad
	pfstartservice_btkad pstart2=(pfstartservice_btkad)GetProcAddress(m_hBTKADMod, L"startservice");
	if(pstart2==NULL || !pstart2(kadport)) return false;

	//start bt
	pfstartservice_bt pstart3=(pfstartservice_bt)GetProcAddress(m_hBTMod, L"startservice");
	if(pstart3==NULL || !pstart3(btport)) return false;

	m_btport=btport; //btkad use it for task

	//make port map if possible
	pfaddportmap_upnp pportmap=(pfaddportmap_upnp)GetProcAddress(m_hUPNPMod, L"addportmap");
	pportmap(btport, true); //tcp portmap
	pportmap(kadport, false);  //udp portmap

	return true;
}

void CPlugInManager::StopServices()
{
	//stop bt
	pfstopservice_bt pstop1=(pfstopservice_bt)GetProcAddress(m_hBTMod, L"stopservice");
	if(pstop1!=NULL) {
		pstop1();
	}

	//stop btkad
	pfstopservice_btkad pstop2=(pfstopservice_btkad)GetProcAddress(m_hBTKADMod, L"stopservice");
	if(pstop2!=NULL) {
		pstop2();
	}

	//stop upnp
	pfstopservice_upnp pstop3=(pfstopservice_upnp)GetProcAddress(m_hUPNPMod, L"stopservice");
	if(pstop3!=NULL) {
		pstop3();
	}

	//stop all job in bt
	DelAllTaskFromBT();

}

void CPlugInManager::AddTaskToKad(char* phash)
{
	pfaddtask_btkad add=(pfaddtask_btkad)GetProcAddress(m_hBTKADMod, L"addtask");
	if(add!=NULL) {
		add(phash, m_btport);
	}
}

void CPlugInManager::DelTaskFromKad(char* phash)
{
	pfremovetask_btkad del=(pfremovetask_btkad)GetProcAddress(m_hBTKADMod, L"removetask");
	if(del!=NULL) {
		del(phash);
	}
}

void CPlugInManager::DelAllTaskFromBT()
{
	pfstopalljob_bt pstop=(pfstopalljob_bt)GetProcAddress(m_hBTMod, L"stopalljob");
	if(pstop!=NULL) {
		pstop();
	}
}

int  CPlugInManager::CreateTaskToBT(int taskid)
{
	pfcreatejob_bt pcreate=(pfcreatejob_bt)GetProcAddress(m_hBTMod, L"createjob");
	if(pcreate!=NULL) {
		return pcreate(taskid);
	}

	return -1;
}

bool CPlugInManager::AddTaskToBT(_NewJobStruct& task)
{
	pfbeginjob_bt pbegin=(pfbeginjob_bt)GetProcAddress(m_hBTMod, L"beginjob");
	if(pbegin!=NULL) {
		return pbegin(&task);
	}

	return false;
}

void CPlugInManager::DelTaskFromBT(int taskid)
{
	pfdeletejob_bt pdel=(pfdeletejob_bt)GetProcAddress(m_hBTMod, L"deletejob");
	if(pdel!=NULL) {
		pdel(taskid);
	}

}

int CPlugInManager::GetPeersFromKad(char* hash, int buflen, char* buf, int* total)
{
	pgetpeers_btkad pgetpeers=(pgetpeers_btkad)GetProcAddress(m_hBTKADMod, L"getpeers");
	if(pgetpeers!=NULL) {
		return pgetpeers(hash, buflen, buf, total);
	}

	return 0;
}

void CPlugInManager::AddPeersToTask(int tid, int nbyte, const char* pdata)
{
	pfaddpeers_bt paddpeers=(pfaddpeers_bt)GetProcAddress(m_hBTMod, L"addpeers");
	if(paddpeers!=NULL) {
		paddpeers(tid, nbyte, pdata);
	}

}


float CPlugInManager::GetProgress(int tid)
{
	pfgetprogress_bt pgetprogress=(pfgetprogress_bt)GetProcAddress(m_hBTMod, L"getprogress");
	if(pgetprogress!=NULL) {
		return pgetprogress(tid);
	}

	return 0.0f;
}

bool CPlugInManager::GetSpeed(int tid, int& dwspd, int& upspd)
{
	pfgetspeed_bt pfgetspeed=(pfgetspeed_bt)GetProcAddress(m_hBTMod, L"getspeed");
	if(pfgetspeed!=NULL) {
		return pfgetspeed(tid, &dwspd, &upspd);
	}

	return false;
}

bool CPlugInManager::GetTaskStatus(int taskid, _JOB_STATUS* status, float* avail)
{
	pfgetstatus_bt pfgetstatus=(pfgetstatus_bt)GetProcAddress(m_hBTMod, L"getstatus");
	if(pfgetstatus!=NULL) {
		return pfgetstatus(taskid, status, avail);
	}
	return false;
}
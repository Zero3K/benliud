/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


/* Created by Anjuta version 1.2.4a */
/*	This file will not be overwritten */

/*
在WINCE的移植中, 我们修改模块的使用方法, 去掉回调结构, 采用查询结构
所有信息都由调用者通过查询来获得, 这样可以简化设计
*/

#include "../../benliud/module_def.h"
#include "../../benliud/service_def.h"
#include "./include/UPnpThread.h"

#include <Tools.h>
#include <wchar.h>
#include <stdio.h>

#if defined(WIN32)||defined(__WXMSW__)||defined(WINCE)

#include <windows.h>  //for messagebox

#define DllExport   __declspec( dllexport )

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#else

#define DllExport

#endif

//call back pointer

static CUPnpThread* pObject=NULL;


UINT	gStateCode=0;	//系统状态码
//10. got ip at local
//20. got no ip at local, start to get by upnp
//30. searching upnp router
//31. found a upnp device Description Url response
//40. connecting description url
//41~49. error when fetch control url
//50. got control url ok
//60. got xip by upnp ok
//61. got xip by upnp failed
//62. wait for next circle to get xip

extern "C"
DllExport UINT 	getstate()
{
	return gStateCode;  //this module is a service module 
}


extern "C" 
DllExport _MODULE_TYPE 	modtype()
{
	return _MOD_SERVICE;  //this module is a service module 
}

extern "C" 
DllExport _SERVICE_TYPE srvtype()
{
	return _SERVICE_UPNP;
}

extern "C" DllExport wchar_t*		moduuid()
{
	static wchar_t* uuid=L"1e8432c0-edc6-11dc-95ff-0800200c9a66";
	return uuid;
}

extern "C" 
DllExport wchar_t*		modvers()
{
	static wchar_t* ver=L"0.3.8.0 for Windows Mobile (arm) [2010/02/10]";
	return ver;
}

extern "C" 
DllExport wchar_t* 	moddesc()
{
	static wchar_t* desc= 
		L"This module provide UPNP service. Some P2P protocol modules "
		L"need it to get extern ip address and make port map on the router.";

	return desc;
}

extern "C" 
DllExport wchar_t* 	modauth()
{
	static wchar_t* auth=L"liubin,China";
	return auth;
}

//is the service running?
extern "C" 
DllExport bool isrunning(unsigned short* port)
{
	return pObject!=NULL;
}

//start the service on ip:port
extern "C" 
DllExport bool startservice()
{

	if(pObject!=NULL) return true;


	pObject=new CUPnpThread();

	if(pObject->Start()) 
	{
		return true;
	}
	else 
	{
		delete pObject;
		pObject=NULL;

		return false;
	}
}



extern "C" 
DllExport void stopservice()
{
	if(pObject==NULL) return;

	pObject->Stop();
	delete pObject;
	pObject=NULL;
	return;
}

extern "C" 
DllExport bool getexternip(char* ipbuf)
{
	if(pObject==NULL) return false;
	return pObject->GetExternIP(ipbuf);
}

extern "C" 
DllExport void addportmap(unsigned short port,bool tcp)
{
	if(pObject==NULL) return;
	pObject->addPortMap(port,tcp?"TCP":"UDP");
}

extern "C" 
DllExport void delportmap(unsigned short port,bool tcp)
{
	if(pObject==NULL) return;
	pObject->removePortMap(port,tcp?"TCP":"UDP");
}




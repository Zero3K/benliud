/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


/* Created by Anjuta version 1.2.4a */
/*	This file will not be overwritten */

#include "../../benliud/include/module_def.h"
#include "../../benliud/include/service_def.h"
#include "./include/DHTThread.h"

#include <wchar.h>
#include <stdio.h>

#if defined(WIN32)||defined(__WXMSW__)||defined(WINCE)

#include <windows.h>  

#define DllExport   __declspec( dllexport )

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#else

#include <netinet/in.h>
#include <arpa/inet.h>

#define DllExport

#endif

static wchar_t gSavePath[MAX_PATH]={0};
static CDHTThread*	pObject=NULL;

//args
static bool announce=true;	//announce in dht
static bool asserver=true;	//run as a server
static bool findpeer=true;  //find peer by dht
static unsigned short level=5;	//运行级别，分1-5个等级
static unsigned short lport=0;


extern "C" DllExport 
_MODULE_TYPE 	modtype()
{
	return _MOD_SERVICE;  //this module is a service module 
}

extern "C" DllExport 
_SERVICE_TYPE srvtype()
{
	return _SERVICE_BTKAD;
}

extern "C" DllExport 
wchar_t*		moduuid()
{
	return L"064d50b0-edc6-11dc-95ff-0800200c9a66";
}

extern "C" DllExport 
wchar_t*		modvers()
{
	return L"0.3.4.0[2010/2/10]";
}

extern "C" DllExport 
wchar_t* 	moddesc()
{
	return L"DHT service module for windows mobile.";
}

extern "C" DllExport 
wchar_t* 	modauth()
{
	return L"liubin,China";
}

//is the service running?
extern "C" DllExport 
bool isrunning(unsigned short* port)
{
	if( pObject!=NULL )
	{
		*port=pObject->GetListenPort();
		return true;
	}

	return false;
}

//start the service on ip:port
extern "C" DllExport 
bool startservice(unsigned short port)
{

	if(pObject!=NULL) return true;

	pObject=new CDHTThread();
	pObject->SetOptions(findpeer,announce,asserver,level);

	if(pObject->Start(port)) 
	{
		//端口影射交给外部主程序去做
		return true;
	}
	else
	{
		delete pObject;
		pObject=NULL;
		return false;
	}

	return false;
}

//
extern "C" DllExport 
bool addtask( char* hash, unsigned short port)
{

	if(pObject==NULL) return false;
	if(hash==NULL) return false;

	if(port==0)
	{
		pObject->AddTask(hash,false,port); //just get but not put
	}
	else
	{
		pObject->AddTask(hash,true,port);
	}

	return true;
}

extern "C" DllExport 
bool removetask(char* hash)
{
	if(pObject==NULL) return false;

	pObject->RemoveTask(hash);
	return true;
}

extern "C" DllExport 
void stopservice()
{
	if(pObject==NULL) return;

	pObject->Stop();
	delete pObject;
	pObject=NULL;

}


extern "C" DllExport 
void addinitnode(unsigned int iip,unsigned short iport)
{
	pObject->AddInitialNodes(iip,iport);
}

//call to get the result of peers on hash,every result use 6 bytes
//return value is the number of result, total is all the number on hash
extern "C" DllExport 
int getpeers(char* hash, int buflen, char* buf, int* total)
{
	if(pObject==NULL) {
		*total=0;
		return 0;
	}

	return pObject->GetPeers(hash,buflen,buf,total);
}


extern "C" DllExport
void setannounce(bool ann)
{
	announce=ann;	//announce in dht
	if(pObject!=NULL)
	{
		pObject->SetOptions(findpeer,announce,asserver,level);
	}
}

extern "C" DllExport
void setasserver(bool ser)
{
	asserver=ser;	//run as a server
	if(pObject!=NULL)
	{
		pObject->SetOptions(findpeer,announce,asserver,level);
	}
}

extern "C" DllExport
void setfindpeer(bool getpeer)
{
	findpeer=getpeer;  //find peer by dht
	if(pObject!=NULL)
	{
		pObject->SetOptions(findpeer,announce,asserver,level);
	}
}


//运行级别，针对网络情况来调节DHT运行速度
//highest level=5, speed=10p/s, slot=8
//high level=4,speed=8p/s, slot=7
//middle level=3 ,speed=6p/s, slot=6
//low level=2, speed=4p/s, slot=5
//lowest level=1, speed=2p/s, slot=4

extern "C" DllExport
void setlevel(unsigned short lvl)
{
	if(lvl > 10) return;
	level=lvl;

	if(pObject!=NULL)
	{
		pObject->SetOptions(findpeer,announce,asserver,level);
	}
}
/* Created by Anjuta version 1.2.4a */
/*	This file will not be overwritten */

#include "../../benliud/include/module_def.h"
#include "../../benliud/include/service_def.h"
#include "../../benliud/include/callback_def.h"
#include "../../benliud/include/msgtype_def.h"

#include "../include/DHTThread.h"

#include <wchar.h>
#include <stdio.h>

#if defined(WIN32)||defined(__WXMSW__)

#include <windows.h>  //for messagebox

#define DllExport   __declspec( dllexport )

bool __stdcall DllMain( int hModule, 
                       unsigned short  ul_reason_for_call, 
                       void* lpReserved
					 )
{
    return true;
}

#else

#include <netinet/in.h>
#include <arpa/inet.h>

#define DllExport

#endif

//call back pointer
static LOGBACK logback = NULL;
static SERVICEEVENT eventback=NULL;
static UPNPSERVICE	upnpservice=NULL;
static CDHTThread*	pObject=NULL;

//args
static bool announce=true;	//announce in dht
static bool asserver=true;	//run as a server
static bool findpeer=true;  //find peer by dht
static unsigned short level=5;	//运行级别，分1-5个等级
static unsigned short lport=0;

static char savepath[512];

//
//void syslog( std::string info )
//{
//    FILE * fp;
//#ifdef WIN32	
//	fp = fopen( "D:\\benliud-dht.log", "a+" );
//#else
//	fp = fopen( "D:\\benliud-dht.log", "a+" );	
//#endif
//    if ( fp == NULL )
//        return ;
//
//    fprintf( fp, "%s", info.c_str() );
//
//    fclose( fp );
//}


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
	return L"0.3.3.0[2008/12/15]";
}

extern "C" DllExport 
wchar_t* 	moddesc()
{
	return L"DHT service module.";
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
bool startservice(unsigned int port)
{

	if(pObject!=NULL) return true;

	pObject=new CDHTThread();
	pObject->SetLogBack(logback);
	pObject->SetEventBack(eventback);
	pObject->SetOptions(findpeer,announce,asserver,level);
	pObject->SetSavePath(savepath);

	if(pObject->Start(port)) 
	{
		//make upnp map
		if(upnpservice(_CHECK_UPNP, NULL,0,0))
		{
			upnpservice(_ADD_PORTMAP, NULL, (unsigned short)port, false);
		}

		lport=(unsigned short)port;

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

//optional function.
//set the log function call (system provided)
extern "C" DllExport 
void setsyslog( LOGBACK lb )
{
    logback = lb;
}

extern "C" DllExport 
void seteventback(SERVICEEVENT eb)
{
	eventback=eb;
}

extern "C" DllExport 
void setupnp(UPNPSERVICE pupnp)
{
	upnpservice=pupnp;
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

//utf8 savepath
extern "C" DllExport
void setpath(const char* path)
{
	strcpy(savepath,path);
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
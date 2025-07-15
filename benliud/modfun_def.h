/***************************************************************************
 *            modfun_def.h
 *
 *  Thu Nov 30 23:19:52 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/

 
#ifndef _MODFUN_DEF_H
#define _MODFUN_DEF_H

//#include "btpriority_def.h"
#include "service_def.h"
#include "protocol_def.h"
#include "bittorrent_def.h"
#include "module_def.h"
#include "msgtype_def.h"
#include "callback_def.h"

	typedef _MODULE_TYPE	(*pfnmodtype)();
	typedef _PROT_TYPE 		(*pfnprotype)();
	typedef	_SERVICE_TYPE	(*pfnsrvtype)();
	typedef wchar_t*		(*pfnmoduuid)();
	typedef wchar_t*		(*pfnmodvers)();
	typedef wchar_t* 		(*pfnmoddesc)();
	typedef wchar_t* 		(*pfnmodauth)();
	typedef void 			(*pfntheticker)();

//return means:
//0=ok.
//<0 error.
	typedef int			 (*pfnfetchinfo)(int,wchar_t*,wchar_t*,llong*,bool*, wchar_t**, bool*,wchar_t**,int,int,int);
	typedef int			 (*pfnfetchpage)(int,const char*,const char*,llong*,bool*, char**, char**,int,int,int);
	typedef int			 (*pfncheckurl)( const char* ,const char*, llong*, char** );
	typedef void 		 (*pfnlogmsg)(int, _MSGTYPE, int, int, wchar_t* ); 
	typedef int			 (*pfngetjobnum)();
//return means: 
//1=piece finish, change a piece.
//0=task finish, no more work.
//-1=network error.
//-2=write file error.
//-3=need change a url.
	typedef int 		 (*pfncreatejob)(int);
	typedef void 		 (*pfndeletejob)(int);

//bittorrent module func
	typedef int			 (*pfnbeginjob)(int,int, const char*, int, const char* ,const wchar_t*, const wchar_t*,int,int,int,int,_BT_ENCMODE,_BT_STOPMODE,const char*);
	typedef void		 (*pfnstoptask)(int); //for bittorrent module
	typedef void		 (*pfndeletetask)(int); //for bittorrent module
	typedef void		 (*pfnsetactive)(int, bool); //for bittorrent module
	typedef void		 (*pfnadjpriori)( int, const char* );  //for adjust priority 
	typedef void		 (*pfnsetbtkad)(BTKADSERVICE);
	typedef void		 (*pfnsetupnp)(UPNPSERVICE);
	typedef void		 (*pfnadjustupspeed)(int,int);
	typedef void		 (*pfnadjustdwspeed)(int,int);
	typedef void		 (*pfnadjustencrypt)(int,_BT_ENCMODE);
	typedef void		 (*pfnadjustmaxlink)(int,int);
	typedef void		 (*pfnadjustcachesize)(int,int);
	typedef void		 (*pfnadjuststopmode)(int,_BT_STOPMODE);
	typedef void		 (*pfnsetpieceback)( BTTASKPIECE );
	typedef void		 (*pfnsetnameback)( BTTASKNAME );
	typedef void		 (*pfnsetbteventback) (BTTASKEVENT);
	typedef void		 (*pfnbalence) ();
	typedef void		 (*pfnsetchkdir)(CREATEDIR);
	typedef void		 (*pfnsetwcconv)(MB2WCCONV); 
	typedef void		 (*pfnsettrylimit)(int);
	typedef void		 (*pfnsetdwspeed)(int);
	typedef void		 (*pfnsetupspeed)(int);
//btkad service func
	typedef bool		 (*pfnisrunning)(unsigned short* );		//is service running
	typedef bool		 (*pfnstartsrv)(unsigned int);  //start the service on port
	typedef void		 (*pfnstopservice)();
	typedef bool		 (*pfnaddtask)(char*, unsigned short);
	typedef bool		 (*pfndeltask)(char*);
	typedef void		 (*pfnaddnode)(unsigned int, unsigned short);
	typedef int			 (*pfngetpeers)(char*, int, char*, int*);
	typedef void		 (*pfnsetpath)(wchar_t*);
//UPNP service func
	typedef bool		 (*pfngetexternip)(char*);
	typedef void		 (*pfnaddportmap)(unsigned short,bool);
	typedef void		 (*pfndelportmap)(unsigned short,bool);

//service use this to notice status
	typedef void		 (*pfnseteventback)( SERVICEEVENT );

//torrent share service func
	typedef void		 (*pfnsetdbopt)( DBFORTSHARE );
	typedef void		 (*pfnaddclient) (unsigned int , unsigned short);
	typedef void		 (*pfnaddhash) (const char*);
	typedef void		 (*pfndelhash) (const char*);
#endif

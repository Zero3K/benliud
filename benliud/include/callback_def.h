
#ifndef _CALLBACK_DEF_H
#define _CALLBACK_DEF_H

#include "datatype_def.h"
#include "module_def.h"
#include "msgtype_def.h"
#include "protocol_def.h"
#include "proxytype_def.h"
#include "service_def.h"

//for bittorrent
typedef enum{	_SPEED_EVENT,		//use BTTASKEVENT to notice download and upload speed
				_SUMBYTE_EVENT,		//use BTTASKEVENT to notice sum of downloaded byte
				_PEER_EVENT,		//use BTTASKEVENT to notice the connected and all got peers
				_FINISH_PERCENT,	//use BTTASKEVENT to notice the progress of download
				_TTF_EVENT,			//use BTTASKEVENT to notice time to finish event
				_TOTAL_SELECT,		//use BTTASKEVENT to notice the total size and select size
				_AVIAL_EVENT,		//use BTTASKEVENT to notice the current avialbility
				_DOWNLOAD_FINISH,   //use BTTASKEVENT to notice the download finished
				_STOPPED_EVENT,		//use BTTASKEVENT to notice the task have quit
				_FAILED_EVENT,		//use BTTASKEVENT to notice the task failed
				_TOTAL_PEER_EVENT,	//use BTTASKEVENT to notice the total peers
				_FINISHED_PIECE,	//use BTTASKPIECE to notice finished pieces
				_PIECE_SAVE,		//use BTTASKPIECE to notice that save the piece to database
				_PIECE_SUM,			//use BTTASKPIECE to notice the pieces sum form connected peers
				_PRIORITY_SET,		//use BTTASKPIECE to notice the priority set
				_TRACKER_MSG,		//use BTTASKEVENT to notice the tracker msg, the message is only a int value
				_BTDATA_EVENT,		//use BTTASKEVENT to notice the instant data of down and up for speedbar show
				_ACCEPT_EVENT,		//use BTTASKEVENT to notice the accept event happen
				_FILE_PERCENT,		//use BTTASKEVENT to notice the file percent finished
				_PEER_FULLLIST,		//all the peer's info list, transfer with arg3
			} BTEVENT;
//the bittorrent event notice call back
//first int: task id
//second int: event type(speed event,peer event, total peer event)
//third int: download speed/connected peers/total seed
//forth int: upload speed/all peers/total peer
typedef void (*BTTASKEVENT)(int,BTEVENT,llong,llong,void*);
//the bittorrent event notice call back
//this is for piece change notice
//int: taskid
//int: number of piece
//int* the piece sum data
//float: avialbility
typedef void (*BTTASKPIECE)(int,BTEVENT,int,int*,float);
//module use it to notice the taskname!
typedef void (*BTTASKNAME)(int,wchar_t*);

//module use it to check and create directory
//current bt module need it.
typedef bool (*CREATEDIR)(wchar_t*);

//module use it to translate between charset
//current bt module need it.
typedef bool (*MB2WCCONV)(const char*, wchar_t*,int,wchar_t*);

typedef enum{
	_CHECK_BTKAD,
	_ADD_TASK,
	_DEL_TASK,
	_ADD_NODE,
	_GET_PEER,
	_GET_PORT,
}BTKADCALLTYPE;


typedef int  (*BTKADSERVICE)(BTKADCALLTYPE, char* ,char*, int, int* );

typedef enum{
	_CHECK_UPNP, //check
	_GET_EXT_IP,
	_ADD_PORTMAP,
	_DEL_PORTMAP,
}UPNPCALLTYPE;

typedef int  (*UPNPSERVICE)(UPNPCALLTYPE, char* ,unsigned short, bool);

//service event callback
typedef void (*SERVICEEVENT)(_SERVICE_TYPE,_SERVICE_EVENT,int);

typedef enum{
	_GET_TORRENT=0,
	_SET_TORRENT=1,
	_GET_ALLTORRENT=2,
	_QUERY_EXISTS=3,
}DBOPTTYPE;

typedef bool (*DBFORTSHARE)(DBOPTTYPE,char*, int*, char**); //first char* is hash, second is content

typedef bool (*DBFORDHT)(bool, int, unsigned int* , unsigned int* );

typedef void (*MESSAGEBACK)(const char* msg);

#endif

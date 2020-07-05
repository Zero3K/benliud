/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _TRACKER_DEFINES_H
#define _TRACKER_DEFINES_H

#ifndef llong
#ifdef WIN32
typedef __int64 llong;
#else
typedef long long int llong;
#endif
#endif

enum TTrackerState{
    TS_INIT=0,
    TS_CONNECTING,
    TS_REQUESTING,
    TS_ERROR,
    TS_OK
} ;

enum TTrackerEvent{
    TE_START,
    TE_STOP,
    TE_COMPLETE,
    TE_NONE
} ;

enum TTrackerStatus
{
	_TRACKER_WAITING,  //waiting for next update
	_TRACKER_CONNECTING,	//connecting server
	_TRACKER_CONNECTED,		//connected to server
	_TRACKER_SENDED,		//send out our packet
	_TRACKER_RECEIVED,		//received the response
} ;

//tracker report event define

#define TR_NOTRUN		(0)
#define TR_CONNECTING   (-1)
#define TR_CONNECTFAIL (-2)
#define TR_CONNECTTED (-3)
#define TR_HEADERERR  (-4)
#define TR_CONTENTERR	(-5)
#define TR_REQTIMEOUT (-6)
#define TR_GOTERRMSG  (-7)

#endif

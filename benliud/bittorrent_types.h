/***************************************************************************
 *            bittorrent_types.h
 *
 *  Tue May 15 19:37:46 2007
 *  Copyright  2007  User
 *  Email
 ****************************************************************************/
#ifndef _BITTORRENT_TYPES_H
#define _BITTORRENT_TYPES_H

#ifndef WIN32
#define BYTE char
#endif

enum _BT_ENCMODE
{
	_PREFER_ORDINARY=0,
	_PREFER_ENCRYPT=1,
	_ALWAYS_ORDINARY=2,
	_ALWAYS_ENCRYPT=3,
};
enum _BT_STOPMODE
{
	_STOP_MANULLY=0,
	_STOP_FINISH=1,
	_STOP_RATIO1=2,
	_STOP_RATIO2=3,
	_STOP_RATIO3=4,
};

struct _NewJobStruct
{
	int jobid;
	int torsize; //torrent data size
	const char* ptorrent;
	int bitsize; //bitset data size
	const char* pbitset; 
	int prisize; //priority size
	const char* priority;
	const wchar_t* savefolder;	//where to save data file
	UINT codepage;	//encode
	int uplimit;	//upload speed limit in kb/s
	int dwlimit;	//download speed limit in kb/s
	int maxconn;	
	int cache;		//in kb
	_BT_STOPMODE stopmode; //stop mode	

};

#endif

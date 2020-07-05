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

#endif

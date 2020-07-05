/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _BTPRIORITY_DEF_H
#define _BTPRIORITY_DEF_H

#include <list>

struct TPriority
{
	unsigned int Index;
	// [0-7] bit is priority , so priority is in [0-255] , priority==0 is baned
	// [8] bit is preview mode indicator 
	int Priority; 
};

struct TPreview
{
	unsigned int Index;
	bool Preview;
};

typedef std::list<TPriority>	TPriorityList; 

#ifndef llong
#ifdef WIN32
typedef __int64 llong;
#else
typedef long long llong;
#endif
#endif

typedef std::list<llong>		TLengthList;	//every file length ,from 0 to n
#endif



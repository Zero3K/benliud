/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _TSTORAGEFILEINFO_H
#define _TSTORAGEFILEINFO_H

#include <list>


#include <TFileInfo.h> //in bencode lib
#include <Mutex.h>

struct TStorageFileInfo
{
	BencodeLib::TFileInfo fileInfo;
#if defined(WINCE)
	HANDLE handle;
#elif defined(WIN32)
    int handle;
#else
	int handle;
#endif

    int Priority;	//优先级别

	int headindex;	//开始片编号
	int tailindex;	//末尾片编号
	int comppiece;	//已完成的片数，用于计算文件完成度
	int	lastwrite;	//最后写过的位置，全局片位置

};

typedef std::list<TStorageFileInfo> TFileHandleList;

#endif


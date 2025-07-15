/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#ifndef _TSTORAGEFILEINFO_H
#define _TSTORAGEFILEINFO_H

#include <list>


#include <TFileInfo.h> //in bencode lib
#include <Mutex.h>

struct TStorageFileInfo
{
	BencodeLib::TFileInfo fileInfo;
#if defined(WINCE)||defined(WIN32)
	HANDLE handle;
#else
	int handle;
#endif

    int Priority;	//���ȼ���

	int headindex;	//��ʼƬ���
	int tailindex;	//ĩβƬ���
	int comppiece;	//����ɵ�Ƭ�������ڼ����ļ���ɶ�
	int	lastwrite;	//���д����λ�ã�ȫ��Ƭλ��

};

typedef std::list<TStorageFileInfo> TFileHandleList;

#endif


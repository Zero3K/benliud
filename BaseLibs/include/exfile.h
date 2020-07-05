#ifndef _EXFILE_H
#define _EXFILE_H

#include "tools.h"

class CExFile
{
public:
	CExFile();
	virtual ~CExFile();
	bool CreateNewFile(wchar_t* sPathName, llong fsize=0);
	bool OpenOldFile(wchar_t* sPathName, llong fsize=0);
	bool SeekFromBegin(llong pos);
	bool WriteFile(void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten);
	bool ReadFile(void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead);
	static void MakeSureDirectoryExistsForFile(wchar_t* sPathName);
protected:
#if defined( WIN32)||defined(WINCE)
	HANDLE m_hFile;
#else
	int m_hFile;
#endif
};
#endif
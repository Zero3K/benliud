/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/exfile.h"

CExFile::CExFile()
{
#if defined( WIN32)||defined(WINCE)
	m_hFile=INVALID_HANDLE_VALUE;
#else
	m_hFile=-1;
#endif
}

CExFile::~CExFile()
{
#if defined( WIN32)||defined(WINCE)
	if(m_hFile!=INVALID_HANDLE_VALUE)	::CloseHandle(m_hFile);
#else
	if(m_hFile!=-1) close(m_hFile);
#endif
}

bool CExFile::OpenOldFile(wchar_t* sPathName, llong fsize)
{
#if defined( WIN32)||defined(WINCE)
	m_hFile = ::CreateFile( sPathName, GENERIC_READ|GENERIC_WRITE, 
				FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(m_hFile==INVALID_HANDLE_VALUE) return false;

	//old file exists, check if file length ok
	LARGE_INTEGER li;
	li.LowPart=::GetFileSize(m_hFile, (DWORD*)&li.HighPart);

	if(li.LowPart==0xFFFFFFFF && GetLastError() != NO_ERROR)
	{
		//error!
		::CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
		return false;
	}

	if(li.QuadPart==fsize)
	{
		::SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN); //move the pointer back to begin
		return true;
	}
	else
	{
		::CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
		return false;
	}

#else
	return false;
#endif
}

//create the file even an old file exists and set the file length to fsize
bool CExFile::CreateNewFile(wchar_t* sPathName, llong fsize)
{

	MakeSureDirectoryExistsForFile(sPathName);

#if defined( WIN32)||defined(WINCE)
	m_hFile = ::CreateFile( sPathName, GENERIC_READ|GENERIC_WRITE, 
				FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(m_hFile==INVALID_HANDLE_VALUE) return false;

	if(fsize==0) return true;

	LARGE_INTEGER li;
	li.QuadPart=fsize;

	DWORD dwRet=::SetFilePointer(m_hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
	if(dwRet==0xFFFFFFFF && ::GetLastError()!=NO_ERROR)
	{
		::CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
		return false;
	}

	BOOL bRet=::SetEndOfFile(m_hFile);
	if(!bRet) {
		::CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
		return false;
	}

	::SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN); //move the pointer back to begin
	return true;

#else	//non windows
	return false;
#endif
}

bool CExFile::SeekFromBegin(llong pos)
{
#if defined( WIN32)||defined(WINCE)
	if(m_hFile==INVALID_HANDLE_VALUE) return false;

	LARGE_INTEGER li;
	li.QuadPart=pos;

	DWORD dwRet=::SetFilePointer(m_hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
	if(dwRet==0xFFFFFFFF && ::GetLastError()!=NO_ERROR)
	{
		return false;
	}

	return true;

#else	//linux
#endif
}

bool CExFile::WriteFile(void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten)
{
#if defined( WIN32)||defined(WINCE)
	if(m_hFile==INVALID_HANDLE_VALUE) return false;
	return ::WriteFile(m_hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL);
#else
	if(m_hFile==-1) return false;
#endif
}

bool CExFile::ReadFile(void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead)
{
#if defined( WIN32)||defined(WINCE)
	if(m_hFile==INVALID_HANDLE_VALUE) return false;
	return ::ReadFile(m_hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
#else
	if(m_hFile==-1) return false;
#endif
}

void CExFile::MakeSureDirectoryExistsForFile(wchar_t* sPathName)
{
#if defined( WIN32)||defined(WINCE)
	wchar_t* pMove;
	
	wchar_t tempdir[MAX_PATH];


	bool found=false;

	for(int want=2; ;want++)
	{
		int count=0;
		wcscpy(tempdir, sPathName);
		pMove=tempdir;

		found=false;

		while(*pMove)
		{
			if(*pMove==L'\\')
			{
				count++;
				if(count==want)
				{
					*pMove=L'\0';
					::CreateDirectory(tempdir, NULL);
					found=true;
					break;
				}
			}
			
			pMove++;

		}//while

		if(!found) break;
	}
#else
	return;
#endif
}
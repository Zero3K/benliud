// SystemChecker.h: interface for the CSystemChecker class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SYSTEMCHECKER_H
#define _SYSTEMCHECKER_H

//only windows need this module
//mainly for check the connection limit

#ifdef WIN32
#include <wx/wx.h>
#include <windows.h>
#include <wchar.h>

#include "../include/msgtype_def.h"

class CSystemChecker  
{
public:
	unsigned int CheckLinkLimit();
	CSystemChecker();
	virtual ~CSystemChecker();

protected:
	void CheckVersion();
	bool FindLimitInFile(int ver,int& limit);
	bool CheckFileVersion();
	bool IsVista();
	bool IsWinXP();
	bool IsWin2K();
	DWORD m_dwVersion;
	DWORD m_dwMajorVersion;
	DWORD m_dwMinorVersion;
	DWORD m_dwBuild;

	unsigned int   m_nLinkLimit;

	bool  m_bInitialed;
	wchar_t m_TcpIpVer[128];

};
#endif //ifdef WIN32
#endif

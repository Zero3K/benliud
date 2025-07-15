/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliud.h : main header file for the benliud application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resourcesp.h"

// CbenliudApp:
// See benliud.cpp for the implementation of this class
//

#include "PlugInManager.h"

enum _NetType
{
	_Net_NONE,
	_Net_WIFI,
	_Net_GPRS,
	_Net_BLUETOOTH,
	_Net_ASYNC,
};

struct _NetInfo{
	_NetType ntype;
	GUID guidDestNet;
	UCHAR ipv4[4];
};

class CbenliudApp : public CWinApp
{
public:
	CbenliudApp();

// Overrides
public:
	virtual BOOL InitInstance();
	CPlugInManager m_Service;
	_NetType	m_nConnType;
	HANDLE		m_nConnHandle;
// Implementation
public:
	BOOL GetConnectionTypeAndAddr(_NetInfo& info);

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CbenliudApp theApp;

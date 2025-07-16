/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// benliud.h : main header file for the benliud application
//
#pragma once

#include "resourcesp.h"
#include "PlugInManager.h"

// Forward declarations
class CMainFrame;

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

class CbenliudApp
{
public:
	CbenliudApp();
	virtual ~CbenliudApp();

// Operations
public:
	BOOL InitInstance();
	int ExitInstance();
	BOOL GetConnectionTypeAndAddr(_NetInfo& info);

// Implementation
public:
	CPlugInManager* m_Service;
	_NetType	m_nConnType;
	HANDLE		m_nConnHandle;
	CMainFrame* m_pMainWnd;
	HINSTANCE   m_hInstance;

private:
	bool InitializeCommonControls();
	bool InitializeWinsock();
};

extern CbenliudApp theApp;

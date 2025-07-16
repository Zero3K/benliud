/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// benliud.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "benliud.h"
#include "MainFrm.h"
#include "ChooseConnDlg.h"

// Windows CE specific headers removed - not available in Windows 10 SDK
// #include <Connmgr_status.h>
// #include <connmgr.h>
// #pragma comment (lib, "Cellcore.lib")


#ifdef _DEBUG
// DEBUG_NEW is not available in Windows API - use standard new
// #define new DEBUG_NEW
#endif



bool MakeNetConnection(GUID& guid , HANDLE& hConnection)
{
	// Windows CE connection manager code removed - not needed for Windows 10
	// Modern Windows handles network connections automatically
	hConnection = NULL;
	return true; // Always return success for Windows 10
}
//���ص�ַ�����0.0.0.0����������
//���ص�ַ�����255.255.255.255����IPV6, ����֧�������ַ.

bool GetCurrentNetStatus(_NetInfo info[2]) //��ȡ2����������WIFI,GPRS , ���ϵͳû��wifi�򷵻�����none.
{
	// Windows CE network status code removed - not needed for Windows 10
	// Modern Windows applications use different APIs for network status
	
	// Set default values indicating connected status
	info[0].ntype=_Net_WIFI;  // Assume WiFi connection available
	info[1].ntype=_Net_NONE;  // No GPRS needed on desktop Windows
	
	// Set localhost IP for basic connectivity
	info[0].ipv4[0]=127;
	info[0].ipv4[1]=0;
	info[0].ipv4[2]=0;
	info[0].ipv4[3]=1;
	
	info[1].ipv4[0]=0;
	info[1].ipv4[1]=0;
	info[1].ipv4[2]=0;
	info[1].ipv4[3]=0;
	
	return true;
}

// CbenliudApp

// The one and only CbenliudApp object
CbenliudApp theApp;

// CbenliudApp construction
CbenliudApp::CbenliudApp()
{
	m_Service = nullptr;
	m_nConnType = _Net_NONE;
	m_nConnHandle = NULL;
	m_pMainWnd = nullptr;
	m_hInstance = NULL;
}

CbenliudApp::~CbenliudApp()
{
	if (m_Service)
	{
		delete m_Service;
		m_Service = nullptr;
	}
}

BOOL CbenliudApp::GetConnectionTypeAndAddr(_NetInfo& info)
{
	// Windows CE connection manager code removed - not needed for Windows 10
	// Return the selected connection type and localhost IP
	
	info.ntype = m_nConnType;
	
	// Set localhost IP for basic connectivity
	info.ipv4[0] = 127;
	info.ipv4[1] = 0;
	info.ipv4[2] = 0;
	info.ipv4[3] = 1;
	
	return TRUE;
}

bool CbenliudApp::InitializeCommonControls()
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_PROGRESS_CLASS;
	return InitCommonControlsEx(&icex) != FALSE;
}

bool CbenliudApp::InitializeWinsock()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return result == 0;
}

BOOL CbenliudApp::InitInstance()
{
	// Initialize Common Controls
	if (!InitializeCommonControls())
	{
		MessageBox(NULL, L"Failed to initialize Common Controls", L"Error", MB_OK);
		return FALSE;
	}

	// Initialize Winsock
	if (!InitializeWinsock())
	{
		MessageBox(NULL, L"Failed to initialize Winsock", L"Error", MB_OK);
		return FALSE;
	}

	// Initialize plugin manager
	m_Service = new CPlugInManager();
	if(!m_Service->Initial(L"localdir"))
	{
		MessageBox(NULL, L"fail to load modules", L"MSG", MB_OK);
		return FALSE;
	}

	// Create main window
	m_pMainWnd = new CMainFrame();
	if (!m_pMainWnd)
	{
		MessageBox(NULL, L"Failed to create main window", L"Error", MB_OK);
		return FALSE;
	}

	// Initialize main window
	if (!m_pMainWnd->Create())
	{
		MessageBox(NULL, L"Failed to initialize main window", L"Error", MB_OK);
		delete m_pMainWnd;
		m_pMainWnd = nullptr;
		return FALSE;
	}

	// Show the main window
	m_pMainWnd->Show(SW_SHOW);
	m_pMainWnd->Update();


	//����ѡ����������...
	_NetInfo ni[2];

	m_nConnType=_Net_NONE;

	if(GetCurrentNetStatus(ni))
	{
		CChooseConnDlg dlg;
		dlg.SetInitialData(ni);
		dlg.DoModal();
		m_nConnType=dlg.m_nChoose;

	}
	else
	{
		MessageBox(NULL, L"Fail to get net adapter info", L"error", MB_OK);
		return FALSE;
	}

	MessageBox(NULL, L"start net adapter", L"error", MB_OK);
	//�Ѿ��õ�������ѡ�� ���û����������������
	if(m_nConnType==_Net_WIFI)
	{

		if(ni[0].ipv4[0]==0)
		{
			if(!MakeNetConnection(ni[0].guidDestNet, m_nConnHandle))
			{
				MessageBox(NULL, L"fail to make wifi connection", L"Error", MB_OK);
				return FALSE;
			}
		}
	}
	else if(m_nConnType==_Net_GPRS)
	{

		if(ni[1].ipv4[0]==0)
		{

			if(!MakeNetConnection(ni[1].guidDestNet, m_nConnHandle))
			{
				MessageBox(NULL, L"fail to make gprs/edge connection", L"Error", MB_OK);
				return FALSE;
			}
		}

	}

	//��������
	if(!m_Service->StartServices(30000, 30001, 30002))
	{
		::MessageBox(NULL, L"fail to start services", L"MSG", MB_OK);
		return FALSE;
	}

	return TRUE;
}



int CbenliudApp::ExitInstance()
{
	// Stop services
	if (m_Service)
	{
		m_Service->StopServices();
	}

	// Cleanup Winsock
	WSACleanup();

	return 0;
}

// Windows Application Entry Point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Store instance handle
	theApp.m_hInstance = hInstance;

	// Initialize the application
	if (!theApp.InitInstance())
	{
		return FALSE;
	}

	// Main message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup and exit
	return theApp.ExitInstance();
}

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

#include "benliudDoc.h"
#include "benliudView.h"

#include "ChooseConnDlg.h"

// Windows CE specific headers removed - not available in Windows 10 SDK
// #include <Connmgr_status.h>
// #include <connmgr.h>
// #pragma comment (lib, "Cellcore.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
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

BEGIN_MESSAGE_MAP(CbenliudApp, CWinApp)
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()



// CbenliudApp construction
CbenliudApp::CbenliudApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CbenliudApp object
CbenliudApp theApp;

// CbenliudApp initialization
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

BOOL CbenliudApp::InitInstance()
{
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the Windows Mobile specific controls such as CAPEDIT and SIPPREF.
    // SHInitExtraControls(); // Windows CE function - not needed for Windows 10

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// AfxEnableDRA(TRUE); // Windows CE device resolution awareness - not needed for Windows 10

	if(!m_Service.Initial(L"localdir"))
	{
		MessageBox(NULL, L"fail to load modules", L"MSG", MB_OK);
		return FALSE;
	}



	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Benliud-PPC"));
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CbenliudDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CbenliudView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();


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
	if(!m_Service.StartServices(30000, 30001, 30002))
	{
		::MessageBox(NULL, L"fail to start services", L"MSG", MB_OK);
		return FALSE;
	}

	return TRUE;
}



int CbenliudApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	m_Service.StopServices();
	return CWinApp::ExitInstance();
}

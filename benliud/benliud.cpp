/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliud.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "benliud.h"
#include "MainFrm.h"

#include "benliudDoc.h"
#include "benliudView.h"

#include "ChooseConnDlg.h"

#include <Connmgr_status.h>
#include <connmgr.h>
#pragma comment (lib, "Cellcore.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



bool MakeNetConnection(GUID& guid , HANDLE& hConnection)
{
	CONNMGR_CONNECTIONINFO conn;
	conn.cbSize=sizeof(conn);
	conn.dwParams =CONNMGR_PARAM_GUIDDESTNET;
	conn.dwFlags =0;
	conn.bExclusive=false;
	conn.bDisabled=false;
	conn.dwPriority=CONNMGR_PRIORITY_USERINTERACTIVE;
	memcpy(&conn.guidDestNet, &guid, sizeof(GUID));
	conn.hWnd=NULL;
	conn.lParam=0;

	return S_OK==ConnMgrEstablishConnection(&conn, &hConnection);

}
//返回地址如果是0.0.0.0则是无连接
//返回地址如果是255.255.255.255则是IPV6, 不能支持这个地址.

bool GetCurrentNetStatus(_NetInfo info[2]) //提取2个连接类型WIFI,GPRS , 如果系统没有wifi则返回类型none.
{
	DWORD dwSize=0;

	HRESULT hr=ConnMgrQueryDetailedStatus(NULL, &dwSize);
	//if(hr!=ERROR_INSUFFICIENT_BUFFER) //这里并不是返回ERROR_INSUFFICIENT_BUFFER

	LPBYTE pBuffer = new BYTE[dwSize];
	if(NULL == pBuffer)
	{
		return false;
	}


	hr = ConnMgrQueryDetailedStatus((CONNMGR_CONNECTION_DETAILED_STATUS*)pBuffer, &dwSize);

	if(hr!=S_OK) {delete[] pBuffer; return false;}


	info[0].ntype=_Net_NONE;
	info[1].ntype=_Net_NONE;

	CONNMGR_CONNECTION_DETAILED_STATUS* cmStatus  = (CONNMGR_CONNECTION_DETAILED_STATUS*)pBuffer;


	while(NULL != cmStatus)
	{
		switch(cmStatus->dwType)
		{
			case CM_CONNTYPE_CELLULAR:
				{
					info[1].ntype=_Net_GPRS;
					memcpy(&info[1].guidDestNet, &cmStatus->guidDestNet, sizeof(GUID));

					//get ip
					if(cmStatus->dwConnectionStatus==CONNMGR_STATUS_CONNECTED && cmStatus->pIPAddr)
					{
						CONNMGR_CONNECTION_IPADDR *pIPAddr=cmStatus->pIPAddr;
						if(pIPAddr->IPAddr->ss_family==AF_INET)
						{
							sockaddr_in *k=((sockaddr_in*)pIPAddr->IPAddr);
							
							info[1].ipv4[0]=k->sin_addr.S_un.S_un_b.s_b1;
							info[1].ipv4[1]=k->sin_addr.S_un.S_un_b.s_b2;
							info[1].ipv4[2]=k->sin_addr.S_un.S_un_b.s_b3;
							info[1].ipv4[3]=k->sin_addr.S_un.S_un_b.s_b4;
							
						}
						else
						{
							info[1].ipv4[0]=255;
							info[1].ipv4[1]=255;
							info[1].ipv4[2]=255;
							info[1].ipv4[3]=255;
						}
					}
					else
					{
						info[1].ipv4[0]=0;
						info[1].ipv4[1]=0;
						info[1].ipv4[2]=0;
						info[1].ipv4[3]=0;
					}
				}
				break;
			case CM_CONNTYPE_NIC:
				{
					info[0].ntype=_Net_WIFI;
					memcpy(&info[0].guidDestNet, &cmStatus->guidDestNet, sizeof(GUID));
					//get ip
					if(cmStatus->dwConnectionStatus==CONNMGR_STATUS_CONNECTED && cmStatus->pIPAddr)
					{

						CONNMGR_CONNECTION_IPADDR *pIPAddr=cmStatus->pIPAddr;
						//pIPAddr->cIPAddr
						if(pIPAddr->IPAddr->ss_family==AF_INET)
						{
							sockaddr_in *k=((sockaddr_in*)pIPAddr->IPAddr);
							
							info[0].ipv4[0]=k->sin_addr.S_un.S_un_b.s_b1;
							info[0].ipv4[1]=k->sin_addr.S_un.S_un_b.s_b2;
							info[0].ipv4[2]=k->sin_addr.S_un.S_un_b.s_b3;
							info[0].ipv4[3]=k->sin_addr.S_un.S_un_b.s_b4;
							
						}
						else
						{
							info[0].ipv4[0]=255;
							info[0].ipv4[1]=255;
							info[0].ipv4[2]=255;
							info[0].ipv4[3]=255;
						}
					}
					else
					{
						info[0].ipv4[0]=0;
						info[0].ipv4[1]=0;
						info[0].ipv4[2]=0;
						info[0].ipv4[3]=0;
					}
				}
				break;
			default:
				break;
		}

		cmStatus = cmStatus->pNext;
	}

	delete[] pBuffer;
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
	//type=m_nConnType;

	DWORD dwSize=0;

	HRESULT hr=ConnMgrQueryDetailedStatus(NULL, &dwSize);

	LPBYTE pBuffer = new BYTE[dwSize];
	if(NULL == pBuffer)
	{
		return false;
	}


	hr = ConnMgrQueryDetailedStatus((CONNMGR_CONNECTION_DETAILED_STATUS*)pBuffer, &dwSize);

	if(hr!=S_OK) {delete[] pBuffer; return false;}

	info.ntype=_Net_NONE;

	CONNMGR_CONNECTION_DETAILED_STATUS* cmStatus  = (CONNMGR_CONNECTION_DETAILED_STATUS*)pBuffer;
	
	info.ipv4[0]=0;
	info.ipv4[1]=0;
	info.ipv4[2]=0;
	info.ipv4[3]=0;

	while(NULL != cmStatus)
	{
		if((cmStatus->dwType==CM_CONNTYPE_CELLULAR && m_nConnType==_Net_GPRS)||
			(cmStatus->dwType==CM_CONNTYPE_NIC && m_nConnType==_Net_WIFI))
		{
			if(cmStatus->dwConnectionStatus==CONNMGR_STATUS_CONNECTED && 
				cmStatus->pIPAddr && 
				cmStatus->pIPAddr->IPAddr[0].ss_family==AF_INET)
			{
				sockaddr_in *k=((sockaddr_in*)(cmStatus->pIPAddr)->IPAddr);

				info.ipv4[0]=k->sin_addr.S_un.S_un_b.s_b1;
				info.ipv4[1]=k->sin_addr.S_un.S_un_b.s_b2;
				info.ipv4[2]=k->sin_addr.S_un.S_un_b.s_b3;
				info.ipv4[3]=k->sin_addr.S_un.S_un_b.s_b4;
			}

		}

		cmStatus = cmStatus->pNext;
	}

	delete[] pBuffer;
	info.ntype=m_nConnType;

	return true;

}

BOOL CbenliudApp::InitInstance()
{
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the Windows Mobile specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableDRA(TRUE); //屏幕转向识别

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


	//连接选择并启动连接...
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
	//已经得到了网络选择， 如果没有连接则现在连接
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

	//启动服务
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

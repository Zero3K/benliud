/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "benliud.h"

#include "MainFrm.h"

#include <afxdlgs.h>

#include <TorrentFile.h>
#include <BenNode.h>
#include <Tools.h>
#include "SelectFileDlg.h"
#include "SelectEncodingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void syslog( std::string info )
{
    FILE * fp;
    fp = fopen( "\\benliud.log", "a+" );
    if ( fp == NULL )
        return ;

    fprintf( fp, "%s", info.c_str() );

    fclose( fp );
}

//const DWORD dwAdornmentFlags = 0; // exit button
//const DWORD dwAdornmentFlags = CMDBAR_HELP|CMDBAR_OK; // exit button
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_MENU_OPEN, &CMainFrame::OnMenuOpen)
	ON_COMMAND(ID_MENU_QUIT, &CMainFrame::OnMenuQuit)
	ON_WM_TIMER()
	ON_COMMAND(ID_MENU_INFOPANEL, &CMainFrame::OnMenuInfopanel)
	ON_UPDATE_COMMAND_UI(ID_MENU_INFOPANEL, &CMainFrame::OnUpdateMenuInfopanel)
	ON_COMMAND(ID_MENU_STOP, &CMainFrame::OnMenuStop)
	ON_COMMAND(ID_MENU_DELETE, &CMainFrame::OnMenuDelete)
	ON_COMMAND(ID_MENU_CONNECTION, &CMainFrame::OnMenuConnection)
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_nTaskId=0;
	m_bShowInfoPanel=false;
	//ʱ������
	//this->SetTimer(1, 10000, NULL);
	//this->SetTimer(2, 1000, NULL);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	if(!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC)) {
		return -1;
	}

	// Note: No toolbar resource available, create empty toolbar
	// Original code tried to load IDR_MENU1 which is a menu resource

	//if(-1==m_wndToolBar.AddBitMap(IDB_BITMAP1, 3)) {
	//	return -1;
	//}

	//TBBUTTON buts[3];
	//buts[0].iBitmap=0;
	//buts[1].iBitmap=1;
	//buts[2].iBitmap=2;
	//buts[0].fsStyle=TBSTYLE_BUTTON;
	//buts[1].fsStyle=TBSTYLE_BUTTON;
	//buts[2].fsStyle=TBSTYLE_BUTTON;
	//buts[0].fsState=TBSTATE_ENABLED;
	//buts[1].fsState=TBSTATE_ENABLED;
	//buts[2].fsState=TBSTATE_ENABLED;
	//buts[0].dwData=0;
	//buts[1].dwData=0;
	//buts[2].dwData=0;
	//buts[0].iString=-1;
	//buts[1].iString=-1;
	//buts[2].iString=-1;

	//if(!m_wndToolBar.AddButtons(3, buts))
	//{
	//	return -1;
	//}

	//if(!m_wndToolBar.AddAdornments(dwAdornmentFlags))
	//{
	//	return -1;
	//}

	//if (!m_wndToolBar.Create(this) 
	//	||!m_wndToolBar.InsertMenuBar(IDR_MAINFRAME) 
	//	||!m_wndToolBar.AddAdornments(dwAdornmentFlags)
	//	)
	//{
	//	TRACE0("Failed to create CommandBar\n");
	//	return -1;      // fail to create
	//}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_SIZE_FIXED);

	CWnd* pWnd = CWnd::FromHandlePermanent(m_wndToolBar.m_hWnd);

	RECT rect, rectDesktop;
	pWnd->GetWindowRect(&rect);
	pWnd->GetDesktopWindow()->GetWindowRect(&rectDesktop);

	int cx = rectDesktop.right - rectDesktop.left;
	int cy = (rectDesktop.bottom - rectDesktop.top) - (rect.bottom - rect.top);
	this->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);

	//info view
	if(!m_wndInfo.Create(L"InfoViewClassName", L"InfoWindowsName", 
		WS_VISIBLE|WS_CHILD|WS_BORDER, rect, this, IDD_INFOPANEL, NULL))
	{
		return -1;
	}


	this->SetTimer(1, 8000, NULL);
	this->SetTimer(2, 5000, NULL); 
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}



// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}
#endif //_DEBUG

// CMainFrame message handlers




void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//�ڳ�һ��ռ���²�����壿

	if(m_bShowInfoPanel)
	{
		// TODO: Replace Windows CE DRA calls with standard Windows resizing
		// DRA::GetDisplayMode() is Windows CE specific
		//if(DRA::GetDisplayMode() != DRA::Portrait )
		if(cx > cy) // assume landscape if width > height
		{//�����
			CbenliudView* pView=(CbenliudView*)this->GetActiveView();
			pView->MoveWindow(0, 0, cx-80, cy);

			m_wndInfo.MoveWindow(cx-80, 0, 80, cy);
		}
		else
		{//�����
			CbenliudView* pView=(CbenliudView*)this->GetActiveView();
			pView->MoveWindow(0, 0, cx, cy-80);

			m_wndInfo.MoveWindow(0, cy-80, cx, 80);
		}
	}

}

void CMainFrame::OnMenuOpen()
{
	// TODO: Add your command handler code here
	//open a file dialog to select a torrent file.

	WCHAR szFile[MAX_PATH];
	WCHAR szFileTitle[MAX_PATH];

	CString s;

	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.lpstrFile = szFile;
	ofn.lpstrInitialDir=NULL;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Torrent\0*.torrent\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


	if (GetOpenFileName(&ofn)!=TRUE) {

		return;
	}

	HANDLE hf;              // file handle

	
	hf = ::CreateFile(ofn.lpstrFile, GENERIC_READ,
		0, (LPSECURITY_ATTRIBUTES) NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);

	if(hf==INVALID_HANDLE_VALUE) {
		//	MessageBox(L"Open file failed");
		s.LoadStringW(IDS_ERROR_OPENTORRENT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return ;
	}

	DWORD dwLow, dwHigh, dwRead;
	dwLow=GetFileSize(hf, &dwHigh);

	if(dwLow==INVALID_FILE_SIZE && ::GetLastError()!=NO_ERROR)
	{
		CloseHandle(hf);
		s.LoadStringW(IDS_ERROR_OPENTORRENT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	if(dwHigh>0 || dwLow > 2*1024*1024)
	{
		CloseHandle(hf);
		s.LoadStringW(IDS_ERROR_TORRENTTOOBIG);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	if(0xFFFFFFFF==::SetFilePointer(hf, 0, 0, FILE_BEGIN))
	{
		CloseHandle(hf);
		s.LoadStringW(IDS_ERROR_OPENTORRENT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	PBYTE torbuf=new BYTE[dwLow];
	if(!::ReadFile(hf, torbuf, dwLow, &dwRead, NULL))
	{
		delete[] torbuf;
		CloseHandle(hf);

		s.LoadStringW(IDS_ERROR_READTORRENT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	CloseHandle(hf);

	BencodeLib::CTorrentFile tf;
	if(0!=tf.ReadBuf((char*)torbuf, dwLow))
	{
		delete[] torbuf;
		//MessageBox(L"bencode format error.");
		s.LoadStringW(IDS_ERROR_TORRENTFORMAT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}
	
	if(0!=tf.ExtractKeys())
	{
		delete[] torbuf;
		s.LoadStringW(IDS_ERROR_TORRENTFORMAT);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	//����Ƿ����ظ�����
	for(int i=0;i<m_TaskItems.size();i++)
	{
		if(m_TaskItems[i].infohash==tf.GetInfoHash())
		{
			delete[] torbuf;
			MessageBox(L"task already in list or running.");
			return;
		}
	}

	//delete[] torbuf; ���滹��

	UINT encode=65001; //utf8

	CString MainName;
	if(tf.IsUtf8Valid())
	{
		std::string fn=tf.GetName();
		WCHAR ucsname[256];
		Tools::UTF2UCS(fn.data(), ucsname, 256);
		MainName=ucsname;
	}
	else
	{
		std::vector<std::string> names;
		names.push_back(tf.GetName());
		for(int i=0;i<tf.GetFileNumber();i++)
		{
			names.push_back(tf.GetFileName(i));
		}

		
		if(!JudgeCodePage(names, encode))
		{
			delete[] torbuf;
			MessageBox(L"can find codepage for torrent");
			return;
		}

	}


	CSelectFileDlg sdlg;
	WCHAR ucsname[MAX_PATH];
	for(int i=0;i<tf.GetFileNumber();i++)
	{
		std::string s=tf.GetFileName(i);
		Tools::UTF2UCS(s.data(), ucsname, 256);
		sdlg.AddItems(ucsname, true);
	}

	if(IDOK!=sdlg.DoModal())
	{//should not happen
		delete[] torbuf;
		return;
	}

	if(!sdlg.IsAnySelected())
	{
		//MessageBox(L"you didn't select any files, quit");
		delete[] torbuf;

		s.LoadStringW(IDS_ERROR_SELECTNOFILE);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	//������Ҫ���ص������ļ��ߴ缰�ļ����ȼ�
	std::string prios;

	ULONGLONG nAllFileSize=0;
	for(int i=0;i<tf.GetFileNumber();i++)
	{
		if(sdlg.IsSelected(i))
		{
			nAllFileSize+=tf.GetFileLength(i);
			prios.append(1,3);
		}
		else
		{
			prios.append(1,0);
		}
	}

	if(nAllFileSize==0)
	{
		delete[] torbuf;

		s.LoadStringW(IDS_ERROR_SELECTISZERO);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	//��ʾһ������·����
	ofn.hwndOwner=this->GetSafeHwnd();
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L'\0';
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.lpstrFileTitle[0] = L'\0';
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if (!GetOpenFileName(&ofn)) 
	{//it should not happen
		delete[] torbuf;
		return;
	}


	if(wcslen(szFileTitle)==0)
	{
		delete[] torbuf;
		s.LoadStringW(IDS_ERROR_SELECTFOLDER);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}

	ULARGE_INTEGER FreeBytes, TotalBytes, TotalFreeBytes;

	if(!GetDiskFreeSpaceEx( szFile, &FreeBytes, &TotalBytes, &TotalFreeBytes))
	{
		delete[] torbuf;

		s.LoadStringW(IDS_ERROR_GETDISKSPACE);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}


	if(FreeBytes.QuadPart < nAllFileSize)
	{
		delete[] torbuf;

		s.LoadStringW(IDS_ERROR_NOSPACE);
		//s.Format(L"free=%I64d, file=%I64d, not enough space", FreeBytes.QuadPart, nAllFileSize);
		MessageBox(s,0, MB_OK|MB_ICONERROR);
		return;
	}


	//everything ok.
	//add new task item and begin the job.
	//int jobid=theApp.m_Service.CreateTaskToBT(++m_nTaskId);
	//if(jobid<0) {
	//	delete[] torbuf;
	//	s.LoadStringW(IDS_ERROR_CREATETASKFAIL);
	//	MessageBox(s,0, MB_OK|MB_ICONERROR);
	//	return;
	//}

	char utfsavepath[MAX_PATH];
	Tools::UCS2UTF(szFile, utfsavepath, MAX_PATH);

	//add to task list
	_TaskCheckItem ck;
	ck.infohash=tf.GetInfoHash();
	ck.taskid=++m_nTaskId;
	ck.focused=false;
	ck.running=false;
	ck.torrent.append((const char*)torbuf, dwLow);
	ck.savepath=utfsavepath;
	ck.priority=prios;
	m_TaskItems.push_back(ck);

	//add task item for view.
	WCHAR mname[256];
	std::string sname=tf.GetName();
	if(tf.IsUtf8Valid())
	{
		Tools::UTF2UCS(sname.data(), mname, 256);
		((CbenliudView*)this->GetActiveView())->AddNewTaskItem(m_nTaskId, mname);
	}
	else
	{
		CString str;
		Convert(sname.data(), sname.size(), encode, str);
		((CbenliudView*)this->GetActiveView())->AddNewTaskItem(m_nTaskId, str);
	}
	
	delete[] torbuf;

	SheduleTask();

	////add task into bittorrent module.
	//_NewJobStruct newjob;
	//newjob.jobid=jobid;
	//newjob.maxconn=70;
	//newjob.bitsize=0;
	//newjob.pbitset=NULL;
	//newjob.cache=2000;
	//newjob.dwlimit=0;
	//newjob.codepage=encode; 
	//newjob.uplimit=0;
	//newjob.torsize=dwLow;
	//newjob.ptorrent=(const char*)torbuf;
	//newjob.savefolder=szFile;
	//newjob.stopmode=_STOP_FINISH;
	//newjob.prisize=prios.size();
	//newjob.priority=prios.data();

	//bool ok=theApp.m_Service.AddTaskToBT(newjob);
	//if(!ok) {
	//	delete[] torbuf;
	//	s.LoadStringW(IDS_ERROR_STARTJOBFAIL);
	//	MessageBox(s,0, MB_OK|MB_ICONERROR);

	//	return;
	//}


	//delete[] torbuf;


	////add task into btkad module
	//theApp.m_Service.AddTaskToKad((char*)(tf.GetInfoHash().data()));

	////add a timer to check btkad data and update ui



}

void CMainFrame::SheduleTask()
{
	//check count of running
	int count=0;
	for(int i=0;i<m_TaskItems.size();i++)
	{
		if(m_TaskItems[i].running) count++;
	}

	if(count>=3) return;

	for(int i=0;i<m_TaskItems.size() && count<3 ;i++)
	{
		if(!m_TaskItems[i].running) 
		{
			//�������������
			//add task into btkad module
			theApp.m_Service.AddTaskToKad((char*)m_TaskItems[i].infohash.data());
			//add task into bittorrent module.

			int jobid=theApp.m_Service.CreateTaskToBT(m_TaskItems[i].taskid);

			wchar_t szFile[MAX_PATH];
			Tools::UTF2UCS(m_TaskItems[i].savepath.c_str(), szFile, MAX_PATH);

			_NewJobStruct newjob;
			newjob.jobid=jobid;
			newjob.maxconn=70;
			newjob.bitsize=0;
			newjob.pbitset=NULL;
			newjob.cache=2000;
			newjob.dwlimit=0;
			newjob.codepage=m_TaskItems[i].codepage; 
			newjob.uplimit=0;
			newjob.torsize=m_TaskItems[i].torrent.size();
			newjob.ptorrent=m_TaskItems[i].torrent.data();
			newjob.savefolder=szFile;
			newjob.stopmode=_STOP_FINISH;
			newjob.prisize=m_TaskItems[i].priority.size(); 
			newjob.priority=m_TaskItems[i].priority.data();
			bool ok=theApp.m_Service.AddTaskToBT(newjob);
			if(ok) m_TaskItems[i].running=true;
		}
	}

}

void CMainFrame::OnMenuQuit()
{
	// TODO: Add your command handler code here

	theApp.m_Service.StopServices(); //ֹͣ����

	OnClose();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==1)
	{
		//check peers from btkad

		for(int i=0;i<m_TaskItems.size();i++)
		{
			if(!m_TaskItems[i].running) continue;

			int total;
			int peers=theApp.m_Service.GetPeersFromKad((char*)m_TaskItems[i].infohash.data(), 0, NULL, &total);
			if(total >0)
			{
				char *buf=new char[total*6];
				peers=theApp.m_Service.GetPeersFromKad((char*)m_TaskItems[i].infohash.data(), total*6, buf, &total);
				theApp.m_Service.AddPeersToTask(m_TaskItems[i].taskid, peers*6, buf);
				delete[] buf;
			}

		}
	}
	else if(nIDEvent==2)
	{
		//update ui
		for(int i=0;i<m_TaskItems.size();i++)
		{
			if(!m_TaskItems[i].running) continue;
			//״̬+����, �ٷֱ�, �����ٶ�, �ϴ��ٶ�,
			float prog=theApp.m_Service.GetProgress(m_TaskItems[i].taskid);
			((CbenliudView*)this->GetActiveView())->UpdateProgress(m_TaskItems[i].taskid, prog);
			

			int dwspd, upspd;
			if(theApp.m_Service.GetSpeed(m_TaskItems[i].taskid, dwspd, upspd))
			{
				((CbenliudView*)this->GetActiveView())->UpdateSpeed(m_TaskItems[i].taskid, upspd, dwspd);
			}
			else
			{
				((CbenliudView*)this->GetActiveView())->UpdateSpeed(m_TaskItems[i].taskid, -1, -1);
			}

			//�������״̬
			_JOB_STATUS status; float avail;
			if(theApp.m_Service.GetTaskStatus(m_TaskItems[i].taskid, &status, &avail))
			{
				((CbenliudView*)this->GetActiveView())->UpdateStatus(m_TaskItems[i].taskid, status, avail);
			}

		}

	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnMenuInfopanel()
{
	m_bShowInfoPanel=!m_bShowInfoPanel;

	//this->UpdateWindow();
	//this->RedrawWindow();

	//��Ҫһ��onSize�¼�
	CRect r;
	this->GetWindowRect(&r);
	this->MoveWindow(r);
}

void CMainFrame::OnUpdateMenuInfopanel(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowInfoPanel);
}

void CMainFrame::OnMenuStop()
{
	// TODO: Add your command handler code here
	for(int i=0;i<m_TaskItems.size();i++)
	{
		if(m_TaskItems[i].focused && m_TaskItems[i].running)
		{
			//stop btkad work
			//stop bt work
			//delete bt work
		}
	}
}

void CMainFrame::OnMenuDelete()
{
	// TODO: Add your command handler code here
}

bool CMainFrame::JudgeCodePage(std::vector<std::string>& names, UINT& codepage)
{
	CString str;
	bool ok=true;

	for(int i=0;i<names.size();i++)
	{
		if(!Convert(names[i].data(), names[i].size(), 65001, str))
		{
			ok=false;
			break;
		}
	}

	if(ok) {
		codepage=65001;
		return true;
	}

	ok=true;
	for(int i=0;i<names.size();i++)
	{
		if(!Convert(names[i].data(), names[i].size(), 936, str))
		{
			ok=false;
			break;
		}
	}

	if(ok) {
		codepage=936;
		return true;
	}

	ok=true;
	for(int i=0;i<names.size();i++)
	{
		if(!Convert(names[i].data(), names[i].size(), 932, str))
		{
			ok=false;
			break;
		}
	}

	if(ok) {
		codepage=932;
		return true;
	}

	ok=true;
	for(int i=0;i<names.size();i++)
	{
		if(!Convert(names[i].data(), names[i].size(), 950, str))
		{
			ok=false;
			break;
		}
	}

	if(ok) {
		codepage=950;
		return true;
	}

	ok=true;
	for(int i=0;i<names.size();i++)
	{
		if(!Convert(names[i].data(), names[i].size(), 949, str))
		{
			ok=false;
			break;
		}
	}

	if(ok) {
		codepage=949;
		return true;
	}

	return false;
}


bool CMainFrame::Convert(const char* multibyte, int nbytes, UINT codepage, CString& str)
{
	int n;
	wchar_t* wpBuf = NULL;

	n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, 0);

	if(n>0)
	{
		wpBuf=new wchar_t[n+2];
		::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, n);	
		wpBuf[n]=0;

		str=wpBuf;
		delete[] wpBuf;
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class

	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnMenuConnection()
{
	// TODO: Add your command handler code here
	//show the connection infomation
	_NetInfo info;
	if(theApp.GetConnectionTypeAndAddr(info))
	{
		CString show;
		if(info.ntype==_Net_GPRS)
		{
			show+=L"Connection Type: GPRS/EDGE/WAP";
		}
		else if(info.ntype==_Net_WIFI)
		{
			show+=L"Connection Type: WIFI/802.11x";
		}

		if(info.ipv4[0]!=0)
		{
			CString s;
			s.Format(L" IP: %u.%u.%u.%u", info.ipv4[0], info.ipv4[1], info.ipv4[2], info.ipv4[3]);
			show+=s;
		}
		else
		{
			show+=L" IP: None";
		}

		MessageBox(show);
	}
}

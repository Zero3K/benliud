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
#include <TorrentFile.h>
#include <BenNode.h>
#include <Tools.h>
// #include "SelectFileDlg.h"  // TODO: Re-implement without MFC
#include "SelectEncodingDlg.h"

#ifdef _DEBUG
// DEBUG_NEW is not available in Windows API - use standard new
// #define new DEBUG_NEW
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

const wchar_t* CMainFrame::WINDOW_CLASS_NAME = L"BenliudMainFrame";

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_hWnd = NULL;
	m_hTreeView = NULL;
	m_hMainTabControl = NULL;
	m_hListView = NULL;
	m_hToolBar = NULL;
	m_hTabControl = NULL;
	m_hGeneralPanel = NULL;
	m_hFilesPanel = NULL;
	m_hTrackersPanel = NULL;
	m_hOptionsPanel = NULL;
	m_hStatusBar = NULL;
	m_nTaskId = 0;
	m_bShowInfoPanel = FALSE;
	m_wndInfo = nullptr;
	m_wndInfo2 = nullptr;
}

CMainFrame::~CMainFrame()
{
	if (m_wndInfo)
	{
		delete m_wndInfo;
		m_wndInfo = nullptr;
	}
	if (m_wndInfo2)
	{
		delete m_wndInfo2;
		m_wndInfo2 = nullptr;
	}
}

bool CMainFrame::RegisterWindowClass()
{
	// Check if class is already registered
	WNDCLASSEX wcexCheck = {};
	if (GetClassInfoEx(theApp.m_hInstance, WINDOW_CLASS_NAME, &wcexCheck))
	{
		// Class already registered, that's fine
		return true;
	}

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = theApp.m_hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // Use system icon
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);  // Use system icon

	ATOM result = RegisterClassEx(&wcex);
	if (result == 0)
	{
		DWORD error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
		{
			return false;
		}
		// If class already exists, that's ok
		return true;
	}
	
	return true;
}

BOOL CMainFrame::Create()
{
	// Register window class
	if (!RegisterWindowClass())
	{
		MessageBox(NULL, L"Failed to register window class", L"Error", MB_OK);
		return FALSE;
	}

	// Create main window
	m_hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		WINDOW_CLASS_NAME,
		L"Benliud - BitTorrent Client",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL,
		NULL,
		theApp.m_hInstance,
		this);

	if (!m_hWnd)
	{
		DWORD error = GetLastError();
		wchar_t errorMsg[256];
		swprintf_s(errorMsg, L"Failed to create main window. Error: %lu", error);
		MessageBox(NULL, errorMsg, L"Error", MB_OK);
		return FALSE;
	}

	// Note: CreateControls() will be called from OnCreate() during WM_CREATE processing
	// SetupMenu() will also be called from OnCreate()

	return TRUE;
}

void CMainFrame::Show(int nCmdShow)
{
	ShowWindow(m_hWnd, nCmdShow);
}

void CMainFrame::Update()
{
	UpdateWindow(m_hWnd);
}

LRESULT CALLBACK CMainFrame::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMainFrame* pThis = nullptr;

	if (message == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pThis = reinterpret_cast<CMainFrame*>(pCreate->lpCreateParams);
		pThis->m_hWnd = hWnd; // Store the window handle
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<CMainFrame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis)
	{
		return pThis->HandleMessage(message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CMainFrame::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		OnCreate();
		return 0;

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_TIMER:
		OnTimer(wParam);
		return 0;

	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case ID_MENU_OPEN:
				OnMenuOpen();
				break;
			case ID_MENU_QUIT:
				OnMenuQuit();
				break;
			case ID_MENU_INFOPANEL:
				OnMenuInfopanel();
				break;
			case ID_MENU_STOP:
				OnMenuStop();
				break;
			case ID_MENU_DELETE:
				OnMenuDelete();
				break;
			case ID_MENU_CONNECTION:
				OnMenuConnection();
				break;
			default:
				return DefWindowProc(m_hWnd, message, wParam, lParam);
			}
		}
		return 0;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;
			if (pnmh->hwndFrom == m_hTabControl && pnmh->code == TCN_SELCHANGE)
			{
				int selectedTab = TabCtrl_GetCurSel(m_hTabControl);
				ShowTabPanel(selectedTab);
			}
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(m_hWnd, message, wParam, lParam);
	}
}

void CMainFrame::CreateControls()
{
	// Verify that m_hWnd is valid before creating child controls
	if (!m_hWnd || !IsWindow(m_hWnd))
	{
		MessageBox(NULL, L"Invalid parent window handle in CreateControls", L"Error", MB_OK);
		return;
	}

	// Get client rect for layout calculations
	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Create toolbar
	m_hToolBar = CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
		0, 0, width, 30,
		m_hWnd,
		(HMENU)IDC_TOOLBAR,
		theApp.m_hInstance,
		NULL);

	if (m_hToolBar)
	{
		SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		
		// Create image list for toolbar buttons
		HIMAGELIST hToolbarImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 0);
		if (hToolbarImageList)
		{
			// Add icons - using system icons as placeholders
			HICON hOpenIcon = LoadIcon(NULL, IDI_APPLICATION);
			HICON hStartIcon = LoadIcon(NULL, IDI_QUESTION);
			HICON hStopIcon = LoadIcon(NULL, IDI_HAND);
			HICON hDeleteIcon = LoadIcon(NULL, IDI_EXCLAMATION);
			
			if (hOpenIcon) ImageList_AddIcon(hToolbarImageList, hOpenIcon);
			if (hStartIcon) ImageList_AddIcon(hToolbarImageList, hStartIcon);
			if (hStopIcon) ImageList_AddIcon(hToolbarImageList, hStopIcon);
			if (hDeleteIcon) ImageList_AddIcon(hToolbarImageList, hDeleteIcon);
			
			SendMessage(m_hToolBar, TB_SETIMAGELIST, 0, (LPARAM)hToolbarImageList);
			
			// Clean up icons
			if (hOpenIcon) DestroyIcon(hOpenIcon);
			if (hStartIcon) DestroyIcon(hStartIcon);
			if (hStopIcon) DestroyIcon(hStopIcon);
			if (hDeleteIcon) DestroyIcon(hDeleteIcon);
		}
		
		// Add toolbar buttons (matching the images)
		TBBUTTON tbButtons[] = {
			{0, ID_MENU_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Open"},
			{1, ID_MENU_RUN, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Start"},
			{2, ID_MENU_STOP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Stop"},
			{3, ID_MENU_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, (INT_PTR)L"Delete"}
		};
		
		SendMessage(m_hToolBar, TB_ADDBUTTONS, sizeof(tbButtons)/sizeof(TBBUTTON), (LPARAM)tbButtons);
	}

	// Create Tree View (left panel) - 150px wide
	m_hTreeView = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_TREEVIEW,
		L"",
		WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
		0, 30, 150, height - 280,  // Leave space for toolbar, main tabs, bottom panel and status bar
		m_hWnd,
		(HMENU)IDC_TREEVIEW,
		theApp.m_hInstance,
		NULL);

	if (m_hTreeView)
	{
		// Create an image list for tree view icons
		HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 8, 0);
		if (hImageList)
		{
			// Load standard icons or create simple colored squares as placeholders
			HICON hIcon = LoadIcon(NULL, IDI_APPLICATION);
			if (hIcon)
			{
				ImageList_AddIcon(hImageList, hIcon);
				DestroyIcon(hIcon);
			}
			
			// Set the image list for the tree view
			TreeView_SetImageList(m_hTreeView, hImageList, TVSIL_NORMAL);
		}

		// Add tree view items matching the image
		TVINSERTSTRUCT tvInsert = {};
		tvInsert.hParent = TVI_ROOT;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvInsert.item.iImage = 0;
		tvInsert.item.iSelectedImage = 0;

		// Add main categories from the images
		tvInsert.item.pszText = L"Benliud";
		HTREEITEM hBenliud = (HTREEITEM)SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		// Add BitTorrent sub-items
		tvInsert.hParent = hBenliud;
		tvInsert.item.pszText = L"BitTorrent";
		HTREEITEM hBitTorrent = (HTREEITEM)SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Downloaded";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Favorites";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Movie";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Software";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Music";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		
		tvInsert.item.pszText = L"Game";
		SendMessage(m_hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

		// Expand the main items
		SendMessage(m_hTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)hBenliud);
	}

	// Create main tab control (for BitTorrent, File Manager, Local Torrent DB, LogInfo)
	m_hMainTabControl = CreateWindowEx(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_VISIBLE | TCS_TABS,
		150, 30, width - 150, 30,
		m_hWnd,
		(HMENU)IDC_MAIN_TABCONTROL,
		theApp.m_hInstance,
		NULL);

	if (m_hMainTabControl)
	{
		// Add main tabs
		TCITEM tcItem = {};
		tcItem.mask = TCIF_TEXT;
		
		tcItem.pszText = L"BitTorrent";
		TabCtrl_InsertItem(m_hMainTabControl, 0, &tcItem);
		
		tcItem.pszText = L"File Manager";
		TabCtrl_InsertItem(m_hMainTabControl, 1, &tcItem);
		
		tcItem.pszText = L"Local Torrent DB";
		TabCtrl_InsertItem(m_hMainTabControl, 2, &tcItem);
		
		tcItem.pszText = L"LogInfo";
		TabCtrl_InsertItem(m_hMainTabControl, 3, &tcItem);
	}

	// Create ListView control (main torrent list) - below the main tabs
	m_hListView = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		L"",
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
		150, 60, width - 150, height - 280,  // Position after tree view and main tabs
		m_hWnd,
		(HMENU)IDC_LISTVIEW,
		theApp.m_hInstance,
		NULL);

	if (m_hListView)
	{
		// Set extended styles for the ListView
		ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		// Add columns matching the image layout
		LVCOLUMN lvc = {};
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		
		lvc.pszText = L"#";
		lvc.cx = 30;
		ListView_InsertColumn(m_hListView, 0, &lvc);
		
		lvc.pszText = L"FileName";
		lvc.cx = 200;
		ListView_InsertColumn(m_hListView, 1, &lvc);
		
		lvc.pszText = L"Selected[Total] size";
		lvc.cx = 120;
		ListView_InsertColumn(m_hListView, 2, &lvc);
		
		lvc.pszText = L"Down[Up] bytes";
		lvc.cx = 100;
		ListView_InsertColumn(m_hListView, 3, &lvc);
		
		lvc.pszText = L"Progress";
		lvc.cx = 80;
		ListView_InsertColumn(m_hListView, 4, &lvc);
		
		lvc.pszText = L"Begin";
		lvc.cx = 80;
		ListView_InsertColumn(m_hListView, 5, &lvc);
		
		lvc.pszText = L"Down Sp...";
		lvc.cx = 80;
		ListView_InsertColumn(m_hListView, 6, &lvc);
		
		lvc.pszText = L"Up Speed";
		lvc.cx = 80;
		ListView_InsertColumn(m_hListView, 7, &lvc);
		
		lvc.pszText = L"Left Time";
		lvc.cx = 80;
		ListView_InsertColumn(m_hListView, 8, &lvc);
		
		lvc.pszText = L"Peers";
		lvc.cx = 60;
		ListView_InsertColumn(m_hListView, 9, &lvc);
		
		lvc.pszText = L"Copys";
		lvc.cx = 60;
		ListView_InsertColumn(m_hListView, 10, &lvc);
		
		lvc.pszText = L"Torrent";
		lvc.cx = 100;
		ListView_InsertColumn(m_hListView, 11, &lvc);
	}

	// Create bottom tab control panel
	m_hTabControl = CreateWindowEx(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_VISIBLE | TCS_TABS,
		0, height - 220, width, 200,
		m_hWnd,
		(HMENU)IDC_TABCONTROL,
		theApp.m_hInstance,
		NULL);

	if (m_hTabControl)
	{
		// Add tabs matching the images
		TCITEM tcItem = {};
		tcItem.mask = TCIF_TEXT;
		
		tcItem.pszText = L"General";
		TabCtrl_InsertItem(m_hTabControl, 0, &tcItem);
		
		tcItem.pszText = L"Files";
		TabCtrl_InsertItem(m_hTabControl, 1, &tcItem);
		
		tcItem.pszText = L"Trackers";
		TabCtrl_InsertItem(m_hTabControl, 2, &tcItem);
		
		tcItem.pszText = L"Options";
		TabCtrl_InsertItem(m_hTabControl, 3, &tcItem);

		// Create tab content panels (initially only General panel visible)
		CreateTabPanels();
	}

	// Create status bar
	m_hStatusBar = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, height - 20, width, 20,
		m_hWnd,
		(HMENU)IDC_STATUSBAR,
		theApp.m_hInstance,
		NULL);

	if (m_hStatusBar)
	{
		// Set up status bar parts
		int parts[] = {width - 200, width - 100, -1};
		SendMessage(m_hStatusBar, SB_SETPARTS, 3, (LPARAM)parts);
		
		// Set status text matching the images
		SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)L"Ready");
		SendMessage(m_hStatusBar, SB_SETTEXT, 1, (LPARAM)L"● DHT [140]");
		SendMessage(m_hStatusBar, SB_SETTEXT, 2, (LPARAM)L"● 26.85.140.20");
	}
}

void CMainFrame::CreateTabPanels()
{
	if (!m_hTabControl || !IsWindow(m_hTabControl))
		return;

	// Get tab control client area
	RECT tabRect;
	GetClientRect(m_hTabControl, &tabRect);
	TabCtrl_AdjustRect(m_hTabControl, FALSE, &tabRect);

	// Create General panel (tab 0)
	m_hGeneralPanel = CreateWindowEx(
		0,
		L"STATIC",
		L"",
		WS_CHILD | WS_VISIBLE,
		tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top,
		m_hTabControl,
		(HMENU)IDC_GENERAL_PANEL,
		theApp.m_hInstance,
		NULL);

	if (m_hGeneralPanel)
	{
		// Create controls for General tab
		// Completed progress bar
		CreateWindow(L"STATIC", L"Completed:", WS_CHILD | WS_VISIBLE,
			10, 10, 80, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hProgressCompleted = CreateWindow(L"msctls_progress32", NULL, WS_CHILD | WS_VISIBLE,
			100, 10, 200, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hProgressCompleted)
		{
			SendMessage(hProgressCompleted, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(hProgressCompleted, PBM_SETPOS, 100, 0); // 100% complete
		}
		
		CreateWindow(L"STATIC", L"100.00%", WS_CHILD | WS_VISIBLE,
			310, 10, 80, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);

		// Availability progress bar  
		CreateWindow(L"STATIC", L"Availability:", WS_CHILD | WS_VISIBLE,
			10, 40, 80, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hProgressAvailability = CreateWindow(L"msctls_progress32", NULL, WS_CHILD | WS_VISIBLE,
			100, 40, 200, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hProgressAvailability)
		{
			SendMessage(hProgressAvailability, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(hProgressAvailability, PBM_SETPOS, 100, 0); // 100% available
		}

		// Torrent info
		CreateWindow(L"STATIC", L"Torrent:", WS_CHILD | WS_VISIBLE,
			10, 70, 60, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		CreateWindow(L"EDIT", L"C:\\Users\\Bryan\\Downloads\\[WakuTomate] Princess Session Orchestra - 14 (WEB 1080p AVC E-AC3) [4981DE1D].mkv.torrent", 
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
			80, 70, 500, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);

		// Save Path
		CreateWindow(L"STATIC", L"Save Path:", WS_CHILD | WS_VISIBLE,
			10, 100, 60, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		CreateWindow(L"EDIT", L"C:\\Users\\Bryan\\Documents\\benliud\\Downloaded", 
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
			80, 100, 500, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);

		// InfoHash
		CreateWindow(L"STATIC", L"InfoHash:", WS_CHILD | WS_VISIBLE,
			10, 130, 60, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		CreateWindow(L"EDIT", L"4E16C6622A05915ADB8AB075A748C3D42A032C9", 
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
			80, 130, 300, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);

		// Category and Piece info
		CreateWindow(L"STATIC", L"Category: Downloaded", WS_CHILD | WS_VISIBLE,
			400, 130, 150, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
		
		CreateWindow(L"STATIC", L"Piece Len/Num: 512k/1768", WS_CHILD | WS_VISIBLE,
			570, 130, 150, 20, m_hGeneralPanel, NULL, theApp.m_hInstance, NULL);
	}

	// Create Files panel (tab 1)
	m_hFilesPanel = CreateWindowEx(
		0,
		L"STATIC",
		L"",
		WS_CHILD,
		tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top,
		m_hTabControl,
		(HMENU)IDC_FILES_PANEL,
		theApp.m_hInstance,
		NULL);

	if (m_hFilesPanel)
	{
		// Create ListView for files
		HWND hFilesList = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_LISTVIEW,
			L"",
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
			10, 10, tabRect.right - tabRect.left - 20, tabRect.bottom - tabRect.top - 20,
			m_hFilesPanel,
			NULL,
			theApp.m_hInstance,
			NULL);

		if (hFilesList)
		{
			ListView_SetExtendedListViewStyle(hFilesList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			LVCOLUMN lvc = {};
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			
			lvc.pszText = L"ID";
			lvc.cx = 40;
			ListView_InsertColumn(hFilesList, 0, &lvc);
			
			lvc.pszText = L"Priority";
			lvc.cx = 80;
			ListView_InsertColumn(hFilesList, 1, &lvc);
			
			lvc.pszText = L"Preview Mode";
			lvc.cx = 100;
			ListView_InsertColumn(hFilesList, 2, &lvc);
			
			lvc.pszText = L"Size";
			lvc.cx = 100;
			ListView_InsertColumn(hFilesList, 3, &lvc);
			
			lvc.pszText = L"FileName";
			lvc.cx = 400;
			ListView_InsertColumn(hFilesList, 4, &lvc);

			// Add sample file entry
			LVITEM lvi = {};
			lvi.mask = LVIF_TEXT;
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			lvi.pszText = L"0";
			ListView_InsertItem(hFilesList, &lvi);
			
			ListView_SetItemText(hFilesList, 0, 1, L"Normal");
			ListView_SetItemText(hFilesList, 0, 2, L"No");
			ListView_SetItemText(hFilesList, 0, 3, L"883.83 MB");
			ListView_SetItemText(hFilesList, 0, 4, L"[WakuTomate] Princess Session Orchestra - 14 (WEB 1080p AVC E-AC3) [4981DE1D].mkv");
		}
	}

	// Create Trackers panel (tab 2)
	m_hTrackersPanel = CreateWindowEx(
		0,
		L"STATIC",
		L"",
		WS_CHILD,
		tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top,
		m_hTabControl,
		(HMENU)IDC_TRACKERS_PANEL,
		theApp.m_hInstance,
		NULL);

	if (m_hTrackersPanel)
	{
		// Create ListView for trackers
		HWND hTrackersList = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_LISTVIEW,
			L"",
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
			10, 10, tabRect.right - tabRect.left - 20, tabRect.bottom - tabRect.top - 20,
			m_hTrackersPanel,
			NULL,
			theApp.m_hInstance,
			NULL);

		if (hTrackersList)
		{
			ListView_SetExtendedListViewStyle(hTrackersList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			LVCOLUMN lvc = {};
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			
			lvc.pszText = L"ID";
			lvc.cx = 40;
			ListView_InsertColumn(hTrackersList, 0, &lvc);
			
			lvc.pszText = L"Tracker Url";
			lvc.cx = 300;
			ListView_InsertColumn(hTrackersList, 1, &lvc);
			
			lvc.pszText = L"Tracker Response";
			lvc.cx = 300;
			ListView_InsertColumn(hTrackersList, 2, &lvc);

			// Add sample tracker entries
			LVITEM lvi = {};
			lvi.mask = LVIF_TEXT;
			
			// DHT tracker
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			lvi.pszText = L"0";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 0, 1, L"DHT tracker");
			ListView_SetItemText(hTrackersList, 0, 2, L"tracker return 2 peers");

			// Add more trackers
			lvi.iItem = 1;
			lvi.pszText = L"1";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 1, 1, L"http://nyaa.tracker.wf:7777/announce");
			ListView_SetItemText(hTrackersList, 1, 2, L"tracker return 50 peers");

			lvi.iItem = 2;
			lvi.pszText = L"2";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 2, 1, L"udp://open.stealth.si:80/announce");
			ListView_SetItemText(hTrackersList, 2, 2, L"tracker return 120 peers");

			lvi.iItem = 3;
			lvi.pszText = L"3";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 3, 1, L"udp://tracker.opentrackr.org:1337/announce");
			ListView_SetItemText(hTrackersList, 3, 2, L"tracker return 122 peers");

			lvi.iItem = 4;
			lvi.pszText = L"4";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 4, 1, L"udp://exodus.desync.com:6969/announce");
			ListView_SetItemText(hTrackersList, 4, 2, L"tracker return 74 peers");

			lvi.iItem = 5;
			lvi.pszText = L"5";
			ListView_InsertItem(hTrackersList, &lvi);
			ListView_SetItemText(hTrackersList, 5, 1, L"udp://tracker.torrent.eu.org:451/announce");
			ListView_SetItemText(hTrackersList, 5, 2, L"tracker return 123 peers");
		}
	}

	// Create Options panel (tab 3)
	m_hOptionsPanel = CreateWindowEx(
		0,
		L"STATIC",
		L"",
		WS_CHILD,
		tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top,
		m_hTabControl,
		(HMENU)IDC_OPTIONS_PANEL,
		theApp.m_hInstance,
		NULL);

	if (m_hOptionsPanel)
	{
		// Download Speed Limit
		CreateWindow(L"STATIC", L"Download Speed Limit:", WS_CHILD | WS_VISIBLE,
			10, 10, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hDownloadCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			170, 10, 100, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hDownloadCombo)
		{
			SendMessage(hDownloadCombo, CB_ADDSTRING, 0, (LPARAM)L"No Limit");
			SendMessage(hDownloadCombo, CB_ADDSTRING, 0, (LPARAM)L"100 KB/s");
			SendMessage(hDownloadCombo, CB_ADDSTRING, 0, (LPARAM)L"500 KB/s");
			SendMessage(hDownloadCombo, CB_ADDSTRING, 0, (LPARAM)L"1 MB/s");
			SendMessage(hDownloadCombo, CB_ADDSTRING, 0, (LPARAM)L"5 MB/s");
			SendMessage(hDownloadCombo, CB_SETCURSEL, 0, 0); // Select "No Limit"
		}

		// Upload Speed Limit
		CreateWindow(L"STATIC", L"Upload Speed Limit:", WS_CHILD | WS_VISIBLE,
			300, 10, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hUploadCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			460, 10, 100, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hUploadCombo)
		{
			SendMessage(hUploadCombo, CB_ADDSTRING, 0, (LPARAM)L"No Limit");
			SendMessage(hUploadCombo, CB_ADDSTRING, 0, (LPARAM)L"50 KB/s");
			SendMessage(hUploadCombo, CB_ADDSTRING, 0, (LPARAM)L"100 KB/s");
			SendMessage(hUploadCombo, CB_ADDSTRING, 0, (LPARAM)L"500 KB/s");
			SendMessage(hUploadCombo, CB_SETCURSEL, 0, 0); // Select "No Limit"
		}

		// Cache Size
		CreateWindow(L"STATIC", L"Cache Size:", WS_CHILD | WS_VISIBLE,
			10, 50, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hCacheCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			170, 50, 100, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hCacheCombo)
		{
			SendMessage(hCacheCombo, CB_ADDSTRING, 0, (LPARAM)L"1 M");
			SendMessage(hCacheCombo, CB_ADDSTRING, 0, (LPARAM)L"5 M");
			SendMessage(hCacheCombo, CB_ADDSTRING, 0, (LPARAM)L"10 M");
			SendMessage(hCacheCombo, CB_ADDSTRING, 0, (LPARAM)L"20 M");
			SendMessage(hCacheCombo, CB_ADDSTRING, 0, (LPARAM)L"50 M");
			SendMessage(hCacheCombo, CB_SETCURSEL, 2, 0); // Select "10 M"
		}

		// Connection Limit
		CreateWindow(L"STATIC", L"Connection Limit:", WS_CHILD | WS_VISIBLE,
			300, 50, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hConnectionCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			460, 50, 100, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hConnectionCombo)
		{
			SendMessage(hConnectionCombo, CB_ADDSTRING, 0, (LPARAM)L"50");
			SendMessage(hConnectionCombo, CB_ADDSTRING, 0, (LPARAM)L"100");
			SendMessage(hConnectionCombo, CB_ADDSTRING, 0, (LPARAM)L"120");
			SendMessage(hConnectionCombo, CB_ADDSTRING, 0, (LPARAM)L"200");
			SendMessage(hConnectionCombo, CB_ADDSTRING, 0, (LPARAM)L"500");
			SendMessage(hConnectionCombo, CB_SETCURSEL, 2, 0); // Select "120"
		}

		// Encryption Mode
		CreateWindow(L"STATIC", L"Encryption Mode:", WS_CHILD | WS_VISIBLE,
			10, 90, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hEncryptionCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			170, 90, 150, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hEncryptionCombo)
		{
			SendMessage(hEncryptionCombo, CB_ADDSTRING, 0, (LPARAM)L"Prefer encryption connect");
			SendMessage(hEncryptionCombo, CB_ADDSTRING, 0, (LPARAM)L"Require encryption");
			SendMessage(hEncryptionCombo, CB_ADDSTRING, 0, (LPARAM)L"Disable encryption");
			SendMessage(hEncryptionCombo, CB_SETCURSEL, 0, 0); // Select "Prefer encryption connect"
		}

		// Stop Task When
		CreateWindow(L"STATIC", L"Stop Task When:", WS_CHILD | WS_VISIBLE,
			350, 90, 150, 20, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		HWND hStopCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			500, 90, 150, 100, m_hOptionsPanel, NULL, theApp.m_hInstance, NULL);
		
		if (hStopCombo)
		{
			SendMessage(hStopCombo, CB_ADDSTRING, 0, (LPARAM)L"Manually Stop");
			SendMessage(hStopCombo, CB_ADDSTRING, 0, (LPARAM)L"Completed");
			SendMessage(hStopCombo, CB_ADDSTRING, 0, (LPARAM)L"Seeding Ratio 1.0");
			SendMessage(hStopCombo, CB_ADDSTRING, 0, (LPARAM)L"Seeding Ratio 2.0");
			SendMessage(hStopCombo, CB_SETCURSEL, 0, 0); // Select "Manually Stop"
		}
	}

	// Show General panel by default
	ShowTabPanel(0);
}

void CMainFrame::ShowTabPanel(int tabIndex)
{
	// Hide all panels first
	if (m_hGeneralPanel) ShowWindow(m_hGeneralPanel, SW_HIDE);
	if (m_hFilesPanel) ShowWindow(m_hFilesPanel, SW_HIDE);
	if (m_hTrackersPanel) ShowWindow(m_hTrackersPanel, SW_HIDE);
	if (m_hOptionsPanel) ShowWindow(m_hOptionsPanel, SW_HIDE);

	// Show the selected panel
	switch (tabIndex)
	{
	case 0:
		if (m_hGeneralPanel) ShowWindow(m_hGeneralPanel, SW_SHOW);
		break;
	case 1:
		if (m_hFilesPanel) ShowWindow(m_hFilesPanel, SW_SHOW);
		break;
	case 2:
		if (m_hTrackersPanel) ShowWindow(m_hTrackersPanel, SW_SHOW);
		break;
	case 3:
		if (m_hOptionsPanel) ShowWindow(m_hOptionsPanel, SW_SHOW);
		break;
	}
}

void CMainFrame::SetupMenu()
{
	HMENU hMenu = CreateMenu();
	HMENU hFileMenu = CreatePopupMenu();
	
	AppendMenu(hFileMenu, MF_STRING, ID_MENU_OPEN, L"&Open");
	AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hFileMenu, MF_STRING, ID_MENU_QUIT, L"&Quit");
	
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
	
	SetMenu(m_hWnd, hMenu);
}

void CMainFrame::OnCreate()
{
	// Create controls and setup menu now that window handle is valid
	CreateControls();
	SetupMenu();

	// Initialize info panels
	// TODO: Implement info panel creation properly
	/*
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	
	//info view
	if(!m_wndInfo.Create(L"InfoViewClassName", L"InfoWindowsName", 
		WS_VISIBLE|WS_CHILD|WS_BORDER, rect, this, IDD_INFOPANEL, NULL))
	{
		// Handle error
	}
	*/

	// Set up timers
	SetTimer(m_hWnd, 1, 10000, NULL);  // 10 second timer
	SetTimer(m_hWnd, 2, 1000, NULL);   // 1 second timer

	// Add sample torrent data to ListView
	if (m_hListView && IsWindow(m_hListView))
	{
		LVITEM lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.iItem = 0;
		lvi.iSubItem = 0;
		lvi.pszText = L"1";
		ListView_InsertItem(m_hListView, &lvi);
		
		ListView_SetItemText(m_hListView, 0, 1, L"[WakuTomate] Princess Session Orchestra - 14 (WEB 1080p AVC E-AC3) [4981DE1D].mkv");
		ListView_SetItemText(m_hListView, 0, 2, L"883.83 MB");
		ListView_SetItemText(m_hListView, 0, 3, L"902.52 MB[0 B]");
		ListView_SetItemText(m_hListView, 0, 4, L"100.00%");
		ListView_SetItemText(m_hListView, 0, 5, L"20:59:44");
		ListView_SetItemText(m_hListView, 0, 6, L"0 B/s");
		ListView_SetItemText(m_hListView, 0, 7, L"0 B/s");
		ListView_SetItemText(m_hListView, 0, 8, L"Unknown");
		ListView_SetItemText(m_hListView, 0, 9, L"4/0/17/156");
		ListView_SetItemText(m_hListView, 0, 10, L"0.00");
		ListView_SetItemText(m_hListView, 0, 11, L"C:\\Users\\Bryan\\");
	}
}

// CMainFrame message handlers




void CMainFrame::OnSize(int cx, int cy)
{
	// Resize toolbar
	if (m_hToolBar && IsWindow(m_hToolBar))
	{
		MoveWindow(m_hToolBar, 0, 0, cx, 30, TRUE);
	}

	// Resize status bar
	if (m_hStatusBar && IsWindow(m_hStatusBar))
	{
		MoveWindow(m_hStatusBar, 0, cy - 20, cx, 20, TRUE);
		
		// Update status bar parts
		int parts[] = {cx - 200, cx - 100, -1};
		SendMessage(m_hStatusBar, SB_SETPARTS, 3, (LPARAM)parts);
	}

	// Resize tree view (left panel)
	if (m_hTreeView && IsWindow(m_hTreeView))
	{
		MoveWindow(m_hTreeView, 0, 30, 150, cy - 280, TRUE);
	}

	// Resize main tab control
	if (m_hMainTabControl && IsWindow(m_hMainTabControl))
	{
		MoveWindow(m_hMainTabControl, 150, 30, cx - 150, 30, TRUE);
	}

	// Resize main ListView (right panel)
	if (m_hListView && IsWindow(m_hListView))
	{
		MoveWindow(m_hListView, 150, 60, cx - 150, cy - 280, TRUE);
	}

	// Resize bottom tab control
	if (m_hTabControl && IsWindow(m_hTabControl))
	{
		MoveWindow(m_hTabControl, 0, cy - 220, cx, 200, TRUE);
		
		// Update tab panel sizes
		RECT tabRect;
		GetClientRect(m_hTabControl, &tabRect);
		TabCtrl_AdjustRect(m_hTabControl, FALSE, &tabRect);
		
		if (m_hGeneralPanel && IsWindow(m_hGeneralPanel))
		{
			MoveWindow(m_hGeneralPanel, tabRect.left, tabRect.top, 
				tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
		}
		
		if (m_hFilesPanel && IsWindow(m_hFilesPanel))
		{
			MoveWindow(m_hFilesPanel, tabRect.left, tabRect.top, 
				tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
		}
		
		if (m_hTrackersPanel && IsWindow(m_hTrackersPanel))
		{
			MoveWindow(m_hTrackersPanel, tabRect.left, tabRect.top, 
				tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
		}
		
		if (m_hOptionsPanel && IsWindow(m_hOptionsPanel))
		{
			MoveWindow(m_hOptionsPanel, tabRect.left, tabRect.top, 
				tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
		}
	}
}

void CMainFrame::OnMenuOpen()
{
	// TODO: Add your command handler code here
	//open a file dialog to select a torrent file.

	WCHAR szFile[MAX_PATH];
	WCHAR szFileTitle[MAX_PATH];

	std::wstring s;

	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
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
		s = L"Error opening torrent file";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return ;
	}

	DWORD dwLow, dwHigh, dwRead;
	dwLow=GetFileSize(hf, &dwHigh);

	if(dwLow==INVALID_FILE_SIZE && ::GetLastError()!=NO_ERROR)
	{
		CloseHandle(hf);
		s = L"Error opening torrent file";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	if(dwHigh>0 || dwLow > 2*1024*1024)
	{
		CloseHandle(hf);
		s = L"Torrent file too big";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	if(0xFFFFFFFF==::SetFilePointer(hf, 0, 0, FILE_BEGIN))
	{
		CloseHandle(hf);
		s = L"Error opening torrent file";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	PBYTE torbuf=new BYTE[dwLow];
	if(!::ReadFile(hf, torbuf, dwLow, &dwRead, NULL))
	{
		delete[] torbuf;
		CloseHandle(hf);

		s = L"Error reading torrent file";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	CloseHandle(hf);

	BencodeLib::CTorrentFile tf;
	if(0!=tf.ReadBuf((char*)torbuf, dwLow))
	{
		delete[] torbuf;
		//MessageBox(L"bencode format error.");
		s = L"Invalid torrent format";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}
	
	if(0!=tf.ExtractKeys())
	{
		delete[] torbuf;
		s = L"Invalid torrent format";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	//����Ƿ����ظ�����
	for(int i=0;i<m_TaskItems.size();i++)
	{
		if(m_TaskItems[i].infohash==tf.GetInfoHash())
		{
			delete[] torbuf;
			MessageBox(NULL, L"task already in list or running.", L"Warning", MB_OK|MB_ICONWARNING);
			return;
		}
	}

	//delete[] torbuf; ���滹��

	UINT encode=65001; //utf8

	std::wstring MainName;
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
			MessageBox(NULL, L"can find codepage for torrent", L"Error", MB_OK|MB_ICONERROR);
			return;
		}

	}


	// TODO: Re-implement file selection dialog without MFC
	// For now, select all files by default
	/*
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

		s = L"No files selected";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}
	*/

	//������Ҫ���ص������ļ��ߴ缰�ļ����ȼ�
	std::string prios;

	ULONGLONG nAllFileSize=0;
	for(int i=0;i<tf.GetFileNumber();i++)
	{
		// TODO: Re-implement file selection - for now select all files
		if(true) // sdlg.IsSelected(i)
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

		s = L"Selected file size is zero";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	//��ʾһ������·����
	ofn.hwndOwner = m_hWnd;
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
		s = L"Error selecting folder";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}

	ULARGE_INTEGER FreeBytes, TotalBytes, TotalFreeBytes;

	if(!GetDiskFreeSpaceEx( szFile, &FreeBytes, &TotalBytes, &TotalFreeBytes))
	{
		delete[] torbuf;

		s = L"Error getting disk space";
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}


	if(FreeBytes.QuadPart < nAllFileSize)
	{
		delete[] torbuf;

		s = L"Not enough disk space";
		//s.Format(L"free=%I64d, file=%I64d, not enough space", FreeBytes.QuadPart, nAllFileSize);
		MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
		return;
	}


	//everything ok.
	//add new task item and begin the job.
	//int jobid=theApp.m_Service.CreateTaskToBT(++m_nTaskId);
	//if(jobid<0) {
	//	delete[] torbuf;
	//	s.LoadStringW(IDS_ERROR_CREATETASKFAIL);
	//	MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);
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
		// TODO: Re-implement ListView integration
		// ((CbenliudView*)this->GetActiveView())->AddNewTaskItem(m_nTaskId, mname);
	}
	else
	{
		std::wstring str;
		Convert(sname.data(), sname.size(), encode, str);
		// TODO: Re-implement ListView integration  
		// ((CbenliudView*)this->GetActiveView())->AddNewTaskItem(m_nTaskId, str);
	}
	
	delete[] torbuf;

	ScheduleTask();

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
	//	MessageBox(NULL, s.c_str(), L"Error", MB_OK|MB_ICONERROR);

	//	return;
	//}


	//delete[] torbuf;


	////add task into btkad module
	//theApp.m_Service.AddTaskToKad((char*)(tf.GetInfoHash().data()));

	////add a timer to check btkad data and update ui



}

void CMainFrame::ScheduleTask()
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
			theApp.m_Service->AddTaskToKad((char*)m_TaskItems[i].infohash.data());
			//add task into bittorrent module.

			int jobid=theApp.m_Service->CreateTaskToBT(m_TaskItems[i].taskid);

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
			bool ok=theApp.m_Service->AddTaskToBT(newjob);
			if(ok) m_TaskItems[i].running=true;
		}
	}

}

void CMainFrame::OnMenuQuit()
{
	// TODO: Add your command handler code here

	theApp.m_Service->StopServices(); //ֹͣ����

	// Close the main window
	DestroyWindow(m_hWnd);
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
			int peers=theApp.m_Service->GetPeersFromKad((char*)m_TaskItems[i].infohash.data(), 0, NULL, &total);
			if(total >0)
			{
				char *buf=new char[total*6];
				peers=theApp.m_Service->GetPeersFromKad((char*)m_TaskItems[i].infohash.data(), total*6, buf, &total);
				theApp.m_Service->AddPeersToTask(m_TaskItems[i].taskid, peers*6, buf);
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
			//状态+进度, 可得百分比, 下载速度, 上传速度,
			float prog=theApp.m_Service->GetProgress(m_TaskItems[i].taskid);
			// TODO: Re-implement ListView integration
			// ((CbenliudView*)this->GetActiveView())->UpdateProgress(m_TaskItems[i].taskid, prog);
			

			int dwspd, upspd;
			if(theApp.m_Service->GetSpeed(m_TaskItems[i].taskid, dwspd, upspd))
			{
				// TODO: Re-implement ListView integration
				// ((CbenliudView*)this->GetActiveView())->UpdateSpeed(m_TaskItems[i].taskid, upspd, dwspd);
			}
			else
			{
				// TODO: Re-implement ListView integration
				// ((CbenliudView*)this->GetActiveView())->UpdateSpeed(m_TaskItems[i].taskid, -1, -1);
			}

			//检查各种状态
			_JOB_STATUS status; float avail;
			if(theApp.m_Service->GetTaskStatus(m_TaskItems[i].taskid, &status, &avail))
			{
				// TODO: Re-implement ListView integration
				// ((CbenliudView*)this->GetActiveView())->UpdateStatus(m_TaskItems[i].taskid, status, avail);
			}

		}

	}

	// TODO: Converted from MFC - implement Windows API equivalent if needed
	// CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnMenuInfopanel()
{
	m_bShowInfoPanel=!m_bShowInfoPanel;

	//this->UpdateWindow();
	//this->RedrawWindow();

	//需要一个onSize事件
	RECT r;
	GetWindowRect(m_hWnd, &r);
	MoveWindow(m_hWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
}

// MFC UI update handler - commented out since not used in Windows API version
/*
void CMainFrame::OnUpdateMenuInfopanel(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowInfoPanel);
}
*/

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
	std::wstring str;
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


bool CMainFrame::Convert(const char* multibyte, int nbytes, UINT codepage, std::wstring& str)
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

void CMainFrame::OnMenuConnection()
{
	// TODO: Add your command handler code here
	//show the connection infomation
	_NetInfo info;
	if(theApp.GetConnectionTypeAndAddr(info))
	{
		std::wstring show;
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
			std::wstring s;
			wchar_t buffer[64];
			swprintf_s(buffer, L" IP: %u.%u.%u.%u", info.ipv4[0], info.ipv4[1], info.ipv4[2], info.ipv4[3]);
			s = buffer;
			show+=s;
		}
		else
		{
			show+=L" IP: None";
		}

		MessageBox(NULL, show.c_str(), L"Information", MB_OK|MB_ICONINFORMATION);
	}
}

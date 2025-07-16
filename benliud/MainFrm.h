/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include <string>
#include <vector>
#include "InfoPanel.h"
#include "InfoWnd.h"

struct _TaskCheckItem
{
	int taskid;
	bool focused;
	bool running;
	UINT codepage;
	std::string infohash;
	std::string torrent;
	std::string savepath;
	std::string priority;
};

class CMainFrame
{
public:
	CMainFrame();
	virtual ~CMainFrame();

// Operations
public:
	BOOL Create();
	void Show(int nCmdShow);
	void Update();
	
	void SetFocusTask(int taskid)
	{
		for(int i=0;i<m_TaskItems.size();i++)
		{
			if(m_TaskItems[i].taskid!=taskid)
			{
				m_TaskItems[i].focused=false;
			}
			else
			{
				m_TaskItems[i].focused=true;
			}
		}
	}
// Implementation
public:
	HWND m_hWnd;
	HWND m_hTreeView;
	HWND m_hListView;
	HWND m_hToolBar;
	HWND m_hTabControl;
	HWND m_hGeneralPanel;
	HWND m_hFilesPanel;
	HWND m_hTrackersPanel;
	HWND m_hOptionsPanel;
	HWND m_hStatusBar;
	
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	bool JudgeCodePage(std::vector<std::string>& names, UINT& codepage);
	bool Convert(const char* multibyte, int nbytes, UINT codepage, std::wstring& str);
	void ScheduleTask();
	void OnCreate();
	void OnSize(int cx, int cy);
	void OnMenuOpen();
	void OnMenuQuit();
	void OnTimer(UINT_PTR nIDEvent);
	void OnMenuInfopanel();
	void OnMenuStop();
	void OnMenuDelete();
	void OnMenuConnection();
	void CreateTabPanels();
	void ShowTabPanel(int tabIndex);

	CInfoPanel* m_wndInfo;
	CInfoWnd* m_wndInfo2;
	BOOL m_bShowInfoPanel;
	int m_nTaskId;

	std::vector<_TaskCheckItem> m_TaskItems;

private:
	static const wchar_t* WINDOW_CLASS_NAME;
	bool RegisterWindowClass();
	void CreateControls();
	void SetupMenu();
};



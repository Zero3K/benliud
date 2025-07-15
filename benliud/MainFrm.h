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
#include "benliudView.h"
#include "infoPanel.h"
#include "infoWnd.h"

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

class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

public:

// Operations
public:
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
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:  // control bar embedded members
	bool JudgeCodePage(std::vector<std::string>& names, UINT& codepage);
	bool Convert(const char* multibyte, int nbytes, UINT codepage, CString& str);
	void SheduleTask();

	CCommandBar m_wndCommandBar;

	//CSplitterWnd m_wndSplitter;

	CInfoPanel m_wndInfo;
	CInfoWnd *m_wndInfo2;
	BOOL m_bShowInfoPanel;
	int m_nTaskId;

	std::vector<_TaskCheckItem> m_TaskItems;
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMenuOpen();
	afx_msg void OnMenuQuit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMenuInfopanel();
	afx_msg void OnUpdateMenuInfopanel(CCmdUI *pCmdUI);
	afx_msg void OnMenuStop();
	afx_msg void OnMenuDelete();
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
	afx_msg void OnMenuConnection();
};



/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// benliudView.h : interface of the CbenliudView class
//


#pragma once

#include <string>
#include "datatype_def.h"  // For _JOB_STATUS enum

class CbenliudDoc;

class CbenliudView
{
public:
	CbenliudView();
	virtual ~CbenliudView();

// Attributes
public:
	CbenliudDoc* GetDocument() const;
	void UpdateSpeed(int taskid, int upspd, int dwspd);
	void UpdateProgress(int taskid, float prog);
	void RemoveTask(int taskid);

// Operations
public:
	BOOL PreCreateWindow();
	void OnInitialUpdate();
	void AddNewTaskItem(int taskid, const std::wstring& name);
	void UpdateStatus(int taskid, int status, float avail);
	void OnLvnItemchanged(LPNMHDR pNMHDR, LRESULT *pResult);

// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const;
#endif

private:
	CbenliudDoc* m_pDocument;
	HWND m_hListView;  // Handle to the actual ListView control in MainFrame
};


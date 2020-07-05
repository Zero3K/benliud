/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliudView.h : interface of the CbenliudView class
//


#pragma once

class CbenliudDoc;
class CbenliudView : public CListView
{
protected: // create from serialization only
	CbenliudView();
	DECLARE_DYNCREATE(CbenliudView)
	CImageList m_IList;
// Attributes
public:
	CbenliudDoc* GetDocument() const;
	void UpdateSpeed(int taskid, int upspd, int dwspd);
	void UpdateProgress(int taskid, float prog);
	void RemoveTask(int taskid);
// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CbenliudView();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	void AddNewTaskItem(int taskid, CString name);
	void UpdateStatus(int taskid, _JOB_STATUS status, float avail);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in benliudView.cpp
inline CbenliudDoc* CbenliudView::GetDocument() const
   { return reinterpret_cast<CbenliudDoc*>(m_pDocument); }
#endif


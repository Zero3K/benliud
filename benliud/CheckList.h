/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#pragma once


#include <vector>
// CCheckList view

class CCheckList : public CScrollView
{
	friend class CSelectFileDlg;

	DECLARE_DYNCREATE(CCheckList)

protected:
	CCheckList();           // protected constructor used by dynamic creation
	virtual ~CCheckList();

public:
	void AddItem(CString item, BOOL sel=TRUE);
	std::vector<CString> m_StringList;
	std::vector<BOOL> m_SelectList;
	int m_nMaxStringWidth;

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};



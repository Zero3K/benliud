/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#pragma once

#include <vector>
// CCheckWnd

class CCheckWnd : public CWnd
{
	DECLARE_DYNAMIC(CCheckWnd)

public:
	CCheckWnd();
	virtual ~CCheckWnd();
	void AddItem(CString item, BOOL sel=TRUE);
	void ReCalScroll(int cx, int cy);
	bool IsSelected(int itemid);
	bool IsAnySelected();
protected:
	std::vector<CString> m_StringList;
	std::vector<BOOL> m_SelectList;
	int m_nMaxStringWidth;
	int m_nMaxStringHeight;
	int m_nTopLine; //顶部的行序号
	int m_nLeft; //距离左边的像素值
	BOOL m_bVertScroll;
	BOOL m_bHoriScroll;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
//	afx_msg BOOL OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
};



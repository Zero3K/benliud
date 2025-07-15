/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once

#include <vector>
#include <string>
// CCheckWnd

class CCheckWnd
{

public:
	CCheckWnd();
	virtual ~CCheckWnd();
	void AddItem(const std::wstring& item, BOOL sel=TRUE);
	void ReCalScroll(int cx, int cy);
	bool IsSelected(int itemid);
	bool IsAnySelected();
protected:
	std::vector<std::wstring> m_StringList;
	std::vector<BOOL> m_SelectList;
	int m_nMaxStringWidth;
	int m_nMaxStringHeight;
	int m_nTopLine; //�����������
	int m_nLeft; //������ߵ�����ֵ
	BOOL m_bVertScroll;
	BOOL m_bHoriScroll;

	HWND m_hWnd;

	void OnPaint();
	BOOL OnEraseBkgnd(HDC hDC);
	void OnSize(UINT nType, int cx, int cy);
	void OnHScroll(UINT nSBCode, UINT nPos);
	void OnVScroll(UINT nSBCode, UINT nPos);
	void OnMButtonDown(UINT nFlags, POINT point);
	void OnLButtonDown(UINT nFlags, POINT point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, POINT pt);
};



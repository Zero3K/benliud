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
	BOOL SubclassWindow(HWND hWnd);  // Method to attach to existing window
	HWND GetHwnd() const { return m_hWnd; }
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
	WNDPROC m_pfnOriginalWndProc;  // Store original window procedure
	
	// Static window procedure
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	// Instance message handler
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPaint();
	BOOL OnEraseBkgnd(HDC hDC);
	void OnSize(UINT nType, int cx, int cy);
	void OnHScroll(UINT nSBCode, UINT nPos);
	void OnVScroll(UINT nSBCode, UINT nPos);
	void OnMButtonDown(UINT nFlags, POINT point);
	void OnLButtonDown(UINT nFlags, POINT point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, POINT pt);
};



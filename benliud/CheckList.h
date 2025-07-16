/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

该代码基于GPL v2协议发布.

****************************************************************/


#pragma once

#include <vector>
#include <string>

// CCheckList view

class CCheckList
{
	friend class CSelectFileDlg;

public:
	CCheckList();           // constructor
	virtual ~CCheckList();

public:
	void AddItem(const std::wstring& item, BOOL sel=TRUE);
	std::vector<std::wstring> m_StringList;
	std::vector<BOOL> m_SelectList;
	int m_nMaxStringWidth;

#ifdef _DEBUG
	void AssertValid() const;
	void Dump() const;
#endif

protected:
	void OnDraw(HDC hDC);      // overridden to draw this view
	void OnInitialUpdate();     // first time after construct

public:
	BOOL OnEraseBkgnd(HDC hDC);
protected:
	void OnUpdate();
public:
	void OnSize(UINT nType, int cx, int cy);
};
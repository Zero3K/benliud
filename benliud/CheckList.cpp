/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

该代码基于GPL v2协议发布.

****************************************************************/

#include "stdafx.h"
#include "benliud.h"
#include "CheckList.h"

// CCheckList

CCheckList::CCheckList()
{
	m_nMaxStringWidth = 0;
}

CCheckList::~CCheckList()
{
}

// CCheckList drawing

void CCheckList::OnDraw(HDC hDC)
{
	// TODO: add draw code for native data here
}

void CCheckList::OnInitialUpdate()
{
	// TODO: calculate the total size of this view
}

#ifdef _DEBUG
void CCheckList::AssertValid() const
{
	// Validation logic here
}

void CCheckList::Dump() const
{
	// Dump object state
}
#endif //_DEBUG

// CCheckList message handlers

void CCheckList::AddItem(const std::wstring& item, BOOL sel)
{
	m_StringList.push_back(item);
	m_SelectList.push_back(sel);
}

BOOL CCheckList::OnEraseBkgnd(HDC hDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CCheckList::OnUpdate()
{
	// TODO: Add your specialized code here and/or call the base class
}

void CCheckList::OnSize(UINT nType, int cx, int cy)
{
	// TODO: Add your message handler code here
}
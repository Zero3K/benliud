/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// InfoPanel.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "InfoPanel.h"


// CInfoPanel

CInfoPanel::CInfoPanel()
{
	m_hWnd = NULL;
}

CInfoPanel::~CInfoPanel()
{
}

// CInfoPanel diagnostics

#ifdef _DEBUG
void CInfoPanel::AssertValid() const
{
	// Validation logic here
}
#endif //_DEBUG

// Placeholder implementations for Windows API version
BOOL CInfoPanel::Create(HWND hParentWnd, const RECT& rect, UINT nID)
{
	// TODO: Implement window creation
	return TRUE;
}

LRESULT CALLBACK CInfoPanel::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Implement window procedure
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CInfoPanel::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Implement message handling
	return 0;
}

// InfoWnd.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "InfoWnd.h"


// CInfoWnd

CInfoWnd::CInfoWnd()
{
	m_hWnd = NULL;
}

CInfoWnd::~CInfoWnd()
{
}

// Placeholder implementations for Windows API version
BOOL CInfoWnd::Create(HWND hParentWnd, const RECT& rect, UINT nID)
{
	// TODO: Implement window creation
	return TRUE;
}

LRESULT CALLBACK CInfoWnd::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Implement window procedure
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CInfoWnd::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Implement message handling
	return 0;
}

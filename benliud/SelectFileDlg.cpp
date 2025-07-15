/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

协议发布GPL v2协议发布.

****************************************************************/


// SelectFileDlg.cpp : implementation file converted to Windows API
//

#include "stdafx.h"
#include "benliud.h"
#include "SelectFileDlg.h"


// CSelectFileDlg dialog

CSelectFileDlg::CSelectFileDlg(HWND hParent /*=NULL*/)
{
	m_hWnd = NULL;
	m_hParent = hParent;
}

CSelectFileDlg::~CSelectFileDlg()
{
}

BOOL CSelectFileDlg::IsSelected(int itemid)
{
	return m_CheckWnd.IsSelected(itemid); 
}

BOOL CSelectFileDlg::IsAnySelected()
{
	return m_CheckWnd.IsAnySelected();
}

void CSelectFileDlg::AddItems(const std::wstring& item, BOOL sel)
{
	m_StringList.push_back(item);
	m_SelectList.push_back(sel);
}

INT_PTR CSelectFileDlg::DoModal()
{
	return DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD), m_hParent, DialogProc, reinterpret_cast<LPARAM>(this));
}

BOOL CSelectFileDlg::Create(HWND hParentWnd)
{
	m_hParent = hParentWnd;
	m_hWnd = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD), hParentWnd, DialogProc, reinterpret_cast<LPARAM>(this));
	return (m_hWnd != NULL);
}

// Static dialog procedure
INT_PTR CALLBACK CSelectFileDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSelectFileDlg* pThis = NULL;

	if (message == WM_INITDIALOG)
	{
		pThis = reinterpret_cast<CSelectFileDlg*>(lParam);
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		pThis->m_hWnd = hDlg;
	}
	else
	{
		pThis = reinterpret_cast<CSelectFileDlg*>(GetWindowLongPtr(hDlg, DWLP_USER));
	}

	if (pThis)
	{
		return pThis->HandleMessage(hDlg, message, wParam, lParam);
	}

	return FALSE;
}

// Instance dialog procedure
INT_PTR CSelectFileDlg::HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return OnInitDialog(hDlg);

	case WM_SIZE:
		OnSize(hDlg, wParam, LOWORD(lParam), HIWORD(lParam));
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

// CSelectFileDlg message handlers

BOOL CSelectFileDlg::OnInitDialog(HWND hDlg)
{
	// Create and initialize the check window
	HWND hCheckList = GetDlgItem(hDlg, IDC_CHECKLIST);
	if (hCheckList)
	{
		if (!m_CheckWnd.SubclassWindow(hCheckList))
		{
			return FALSE;
		}

		for (int i = 0; i < m_StringList.size(); i++)
		{
			m_CheckWnd.AddItem(m_StringList[i], m_SelectList[i]);
		}
	}

	return TRUE;
}

void CSelectFileDlg::OnSize(HWND hDlg, UINT nType, int cx, int cy)
{
	// Resize the check window to fill the dialog
	HWND hCheckList = GetDlgItem(hDlg, IDC_CHECKLIST);
	if (hCheckList)
	{
		RECT rect;
		GetClientRect(hDlg, &rect);
		
		// Add some padding
		rect.left += 2;
		rect.top += 2;
		rect.right -= 2;
		rect.bottom -= 2;
		
		MoveWindow(hCheckList, rect.left, rect.top, 
			rect.right - rect.left, rect.bottom - rect.top, TRUE);

		// Send size message to the check window
		SendMessage(hCheckList, WM_SIZE, SIZE_RESTORED, 
			MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));
	}
}
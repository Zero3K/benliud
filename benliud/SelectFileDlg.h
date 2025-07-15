/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

This code is published under GPL v2 license.

****************************************************************/


#pragma once

#include "CheckList.h"
#include "CheckWnd.h"
// CSelectFileDlg dialog converted to Windows API
#include <vector>

class CSelectFileDlg
{
public:
	CSelectFileDlg(HWND hParent = NULL);   // standard constructor
	virtual ~CSelectFileDlg();
	void AddItems(const std::wstring& item, BOOL sel);
	BOOL IsSelected(int itemid);
	BOOL IsAnySelected();
	CCheckWnd m_CheckWnd;

	std::vector<std::wstring> m_StringList;
	std::vector<BOOL> m_SelectList;

// Dialog Data
	enum { IDD = IDD_SELECT_FILE_DLG };

	// Windows API methods
	INT_PTR DoModal();
	BOOL Create(HWND hParentWnd = NULL);
	
	HWND GetHwnd() const { return m_hWnd; }

private:
	HWND m_hWnd;
	HWND m_hParent;
	
	// Static dialog procedure
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	
	// Instance dialog procedure
	INT_PTR HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hDlg);
	void OnSize(HWND hDlg, UINT nType, int cx, int cy);
};

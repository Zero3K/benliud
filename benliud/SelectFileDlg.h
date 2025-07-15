/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once

#include "CheckList.h"
#include "CheckWnd.h"
// CSelectFileDlg dialog
#include <vector>

class CSelectFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectFileDlg)

public:
	CSelectFileDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectFileDlg();
	void AddItems(const std::wstring& item, BOOL sel);
	BOOL IsSelected(int itemid);
	BOOL IsAnySelected();
	CCheckWnd m_CheckWnd;

	std::vector<std::wstring> m_StringList;
	std::vector<BOOL> m_SelectList;

// Dialog Data
	enum { IDD = IDD_SELECT_FILE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

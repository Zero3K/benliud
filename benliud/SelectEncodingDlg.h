/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <string>
#include <vector>
// CSelectEncodingDlg dialog

class CSelectEncodingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectEncodingDlg)

public:
	CSelectEncodingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectEncodingDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_ENCODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool Convert(const char* multibyte, int nbytes, UINT codepage, CString& str);
	bool ConvertAllAndSet(UINT codepage, CString& mainname, CStringArray& fnames);
	std::string m_sMainName;
	std::vector<std::string> m_sFileNames;

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_ctrlEncoding;
	CStatic m_ctrlMainName;
	CListCtrl m_ctrlFileList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void SetMainName(std::string mainname);
	void AddFileName(std::string name);
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeEncoding();
};

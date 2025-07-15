/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#pragma once

#include "benliud.h"
#include "afxwin.h"
// CChooseConnDlg dialog

class CChooseConnDlg : public CDialog
{
	DECLARE_DYNAMIC(CChooseConnDlg)

public:
	CChooseConnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseConnDlg();

// Dialog Data
	enum { IDD = IDD_CHOOSECONN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString s1,s2;
	bool b1, b2;
	_NetType m_nChoose;
	void SetInitialData(_NetInfo data[2]);
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
};

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once

#include "benliud.h"
// CChooseConnDlg dialog

class CChooseConnDlg
{
public:
	CChooseConnDlg(HWND hParent = NULL);
	virtual ~CChooseConnDlg();

// Dialog Data
	enum { IDD = IDD_CHOOSECONN };

// Operations
public:
	INT_PTR DoModal();
	void SetInitialData(_NetInfo data[2]);

// Implementation
protected:
	static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
	
	void OnInitDialog();
	void OnSize(int cx, int cy);
	void OnBnClickedOk();
	void OnBnClickedButton1();

public:
	std::wstring s1, s2;
	bool b1, b2;
	_NetType m_nChoose;

private:
	HWND m_hWnd;
	HWND m_hParent;
	_NetInfo m_data[2];
};

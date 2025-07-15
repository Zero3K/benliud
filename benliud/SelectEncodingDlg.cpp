/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// SelectEncodingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "SelectEncodingDlg.h"


// CSelectEncodingDlg

CSelectEncodingDlg::CSelectEncodingDlg()
{
	m_hDlg = NULL;
}

CSelectEncodingDlg::~CSelectEncodingDlg()
{
}

// TODO: Implement Windows API dialog functionality
INT_PTR CSelectEncodingDlg::DoModal(HWND hParentWnd)
{
	// TODO: Implement modal dialog
	return IDOK;
}

INT_PTR CALLBACK CSelectEncodingDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Implement dialog procedure
	return FALSE;
}


// CSelectEncodingDlg message handlers

void CSelectEncodingDlg::OnSize(UINT nType, int cx, int cy)
{
	// No longer need CDialog::OnSize since we're using Windows API
	
	// TODO: Add your message handler code here
	// Note: MoveWindow calls need to be replaced with SetWindowPos
	// for native Windows API controls
	if (m_ctrlEncoding)
		SetWindowPos(m_ctrlEncoding, NULL, 5, 5, cx-10, 25, SWP_NOZORDER);
	if (m_ctrlMainName)
		SetWindowPos(m_ctrlMainName, NULL, 5, 35, cx-10, 25, SWP_NOZORDER);
	if (m_ctrlFileList)
		SetWindowPos(m_ctrlFileList, NULL, 5, 65, cx-10, cy - 65 -5, SWP_NOZORDER);
}

void CSelectEncodingDlg::SetMainName(std::string mainname)
{
	m_sMainName=mainname;
}

void CSelectEncodingDlg::AddFileName(std::string name)
{
	m_sFileNames.push_back(name);
}

BOOL CSelectEncodingDlg::OnInitDialog()
{
	// TODO: Implement initialization for Windows API version
	/*
	// Original MFC code commented out for compilation
	m_ctrlEncoding.AddString(L"UTF-8 [65001]");
	m_ctrlEncoding.AddString(L"BIG5 [950]");
	m_ctrlEncoding.AddString(L"Simplified Chinese [936]");
	m_ctrlEncoding.AddString(L"Japanese [932]");
	m_ctrlEncoding.AddString(L"Korean [949]");
	m_ctrlEncoding.SetExtendedUI();

	std::wstring mainname;
	std::vector<std::wstring> fnames;

	if(ConvertAllAndSet(65001, mainname, fnames))
	{
		this->m_ctrlMainName.SetWindowTextW(mainname.c_str());
		for(int i=0;i<fnames.size();i++)
		{
			this->m_ctrlFileList.InsertItem(i, fnames[i].c_str());
		}
		m_ctrlEncoding.SetCurSel(0);
		return TRUE;
	}
	// ... rest of original implementation
	*/
	return TRUE;
}

void CSelectEncodingDlg::OnCbnSelchangeEncoding()
{
	// TODO: Implement for Windows API version
	/*
	// Original MFC code commented out for compilation
	int x=m_ctrlEncoding.GetCurSel();
	std::wstring mainname;
	std::vector<std::wstring> fnames;
	UINT codepage;
	// ... rest of original implementation
	*/
}

bool CSelectEncodingDlg::ConvertAllAndSet(UINT codepage, std::wstring& mainname, std::vector<std::wstring>& fnames)
{
	std::wstring str;
	if(Convert(m_sMainName.data(), m_sMainName.size(), codepage, str))
	{
		mainname=str;
	}

	for(int i=0;i<m_sFileNames.size();i++)
	{
		if(Convert(m_sFileNames[i].data(), m_sFileNames[i].size(), codepage, str))
		{
			fnames.push_back(str);
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CSelectEncodingDlg::Convert(const char* multibyte, int nbytes, UINT codepage, std::wstring& str)
{
	int n;
	wchar_t* wpBuf = NULL;

	n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, NULL, 0);

	if(n>0)
	{
		wpBuf=new wchar_t[n];
		n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, n);	
		str = std::wstring(wpBuf, n);
		delete[] wpBuf;
		return true;
	}
	else
	{
		return false;
	}
}
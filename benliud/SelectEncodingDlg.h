/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once

#include <string>
#include <vector>
// CSelectEncodingDlg - converted from MFC to Windows API

class CSelectEncodingDlg
{
public:
	CSelectEncodingDlg();   // standard constructor
	virtual ~CSelectEncodingDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_ENCODE };

	// Windows API methods
	INT_PTR DoModal(HWND hParentWnd = NULL);
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	
protected:
	bool Convert(const char* multibyte, int nbytes, UINT codepage, std::wstring& str);
	bool ConvertAllAndSet(UINT codepage, std::wstring& mainname, std::vector<std::wstring>& fnames);
	std::string m_sMainName;
	std::vector<std::string> m_sFileNames;
	HWND m_hDlg;

public:
	HWND m_ctrlEncoding;
	HWND m_ctrlMainName;
	HWND m_ctrlFileList;
	
	void OnSize(UINT nType, int cx, int cy);
	void SetMainName(std::string mainname);
	void AddFileName(std::string name);
	BOOL OnInitDialog();
	void OnCbnSelchangeEncoding();
};

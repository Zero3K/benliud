/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// SelectEncodingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "SelectEncodingDlg.h"


// CSelectEncodingDlg dialog

IMPLEMENT_DYNAMIC(CSelectEncodingDlg, CDialog)

CSelectEncodingDlg::CSelectEncodingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectEncodingDlg::IDD, pParent)
{

}

CSelectEncodingDlg::~CSelectEncodingDlg()
{
}

void CSelectEncodingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENCODING, m_ctrlEncoding);
	DDX_Control(pDX, IDC_MAINNAME, m_ctrlMainName);
	DDX_Control(pDX, IDC_NAMELIST, m_ctrlFileList);
}


BEGIN_MESSAGE_MAP(CSelectEncodingDlg, CDialog)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_ENCODING, &CSelectEncodingDlg::OnCbnSelchangeEncoding)
END_MESSAGE_MAP()


// CSelectEncodingDlg message handlers

void CSelectEncodingDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	m_ctrlEncoding.MoveWindow(5,5, cx-10, 25);
	m_ctrlMainName.MoveWindow(5,35, cx-10, 25);
	m_ctrlFileList.MoveWindow(5,65, cx-10, cy - 65 -5);
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
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ctrlEncoding.AddString(L"UTF-8 [65001]");
	m_ctrlEncoding.AddString(L"BIG5 [950]");
	m_ctrlEncoding.AddString(L"Simplified Chinese [936]");
	m_ctrlEncoding.AddString(L"Japanese [932]");
	m_ctrlEncoding.AddString(L"Korean [949]");
//	m_ctrlEncoding.ShowDropDown();
	m_ctrlEncoding.SetExtendedUI();


	CString mainname;
	CStringArray fnames;

	if(ConvertAllAndSet(65001, mainname, fnames))
	{
		this->m_ctrlMainName.SetWindowTextW(mainname);
		for(int i=0;i<fnames.GetSize();i++)
		{
			this->m_ctrlFileList.InsertItem(i, fnames[i]);
		}

		m_ctrlEncoding.SetCurSel(0);
		return TRUE;
	}

	fnames.RemoveAll();
	if(ConvertAllAndSet(936, mainname, fnames))
	{
		this->m_ctrlMainName.SetWindowTextW(mainname);
		for(int i=0;i<fnames.GetSize();i++)
		{
			this->m_ctrlFileList.InsertItem(i, fnames[i]);
		}

		m_ctrlEncoding.SetCurSel(2);
		return TRUE;
	}

	fnames.RemoveAll();
	if(ConvertAllAndSet(950, mainname, fnames))
	{
		this->m_ctrlMainName.SetWindowTextW(mainname);
		for(int i=0;i<fnames.GetSize();i++)
		{
			this->m_ctrlFileList.InsertItem(i, fnames[i]);
		}
		m_ctrlEncoding.SetCurSel(1);
		return TRUE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectEncodingDlg::OnCbnSelchangeEncoding()
{
	// TODO: Add your control notification handler code here
	int x=m_ctrlEncoding.GetCurSel();
	CString mainname;
	CStringArray fnames;
	UINT codepage;
	switch(x)
	{
	case 0:
		codepage=65001;
		break;
	case 1:
		codepage=950;
		break;
	case 2:
		codepage=936;
		break;
	case 3:
		codepage=932;
		break;
	case 4:
		codepage=949;
		break;
	default:
		codepage=65001;
		break;
	}

	if(ConvertAllAndSet(codepage, mainname, fnames))
	{
		m_ctrlFileList.DeleteAllItems();
		this->m_ctrlMainName.SetWindowTextW(mainname);
		for(int i=0;i<fnames.GetSize();i++)
		{
			this->m_ctrlFileList.InsertItem(i, fnames[i]);
		}
	}
	else
	{
		MessageBox(L"Not a proper codepage");
	}
}

bool CSelectEncodingDlg::ConvertAllAndSet(UINT codepage, CString& mainname, CStringArray& fnames)
{
	CString str;
	if(Convert(m_sMainName.data(), m_sMainName.size(), codepage, str))
	{
		mainname=str;
	}

	for(int i=0;i<m_sFileNames.size();i++)
	{
		if(Convert(m_sFileNames[i].data(), m_sFileNames[i].size(), codepage, str))
		{
			fnames.Add(str);
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CSelectEncodingDlg::Convert(const char* multibyte, int nbytes, UINT codepage, CString& str)
{
	int n;
	wchar_t* wpBuf;

	n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, 0);

	if(n>0)
	{
		wpBuf=new wchar_t[n];
		n=::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS,  multibyte, nbytes, wpBuf, n);	
		str=wpBuf;
		delete[] wpBuf;
		return true;
	}
	else
	{
		return false;
	}
}
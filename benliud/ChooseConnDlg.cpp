/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// ChooseConnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "ChooseConnDlg.h"


// CChooseConnDlg dialog

IMPLEMENT_DYNAMIC(CChooseConnDlg, CDialog)

CChooseConnDlg::CChooseConnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseConnDlg::IDD, pParent)
{

}

CChooseConnDlg::~CChooseConnDlg()
{
}

void CChooseConnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChooseConnDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_OK, &CChooseConnDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChooseConnDlg message handlers

void CChooseConnDlg::SetInitialData(_NetInfo data[2])
{
	if(data[0].ntype==_Net_NONE)
	{
		s1=L"WIFI (not supported by system)";
		
		b1=false;
	}
	else
	{
		if(data[0].ipv4[0]==0)
		{
			s1=L"WIFI (not connected)";
			b1=true;
		}
		else if(data[0].ipv4[0]==255)
		{
			s1=L"WIFI (ipv6 not supported)";
			b1=false;
		}
		else
		{

			s1.Format(L"WIFI (%u.%u.%u.%u)", 
				data[0].ipv4[0], 
				data[0].ipv4[1],
				data[0].ipv4[2],
				data[0].ipv4[3]);

			b1=true;
		}

	}

	if(data[1].ntype==_Net_NONE)
	{
		s2=L"GPRS/EDGE is not supported by system.";
		b2=false;
	}
	else
	{
		if(data[1].ipv4[0]==0)
		{
			s2=L"GPRS/EDGE (not connected)";
			b2=true;
		}
		else if(data[1].ipv4[0]==255)
		{
			s2=L"GPRS/EDGE (ipv6 not supported)";
			b2=false;
		}
		else
		{

			s2.Format(L"GPRS/EDGE (%u.%u.%u.%u)", 
				data[0].ipv4[0], 
				data[0].ipv4[1],
				data[0].ipv4[2],
				data[0].ipv4[3]);

			b2=true;
		}
	}
}

void CChooseConnDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here

}

BOOL CChooseConnDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	GetDlgItem(IDC_RADIO1)->SetWindowTextW(s1);
	GetDlgItem(IDC_RADIO2)->SetWindowTextW(s2);
	GetDlgItem(IDC_RADIO1)->EnableWindow(b1);
	GetDlgItem(IDC_RADIO2)->EnableWindow(b2);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseConnDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	GetDlgItem(ID_OK)->MoveWindow(0,cy-25, cx, cy);
}

void CChooseConnDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if(BST_CHECKED==((CButton*)GetDlgItem(IDC_RADIO1))->GetCheck())
	{
		m_nChoose=_Net_WIFI;
	}
	else if(BST_CHECKED==((CButton*)GetDlgItem(IDC_RADIO2))->GetCheck())
	{
		m_nChoose=_Net_GPRS;
	}
	else
	{
		m_nChoose=_Net_WIFI;
	}

	OnOK();
}

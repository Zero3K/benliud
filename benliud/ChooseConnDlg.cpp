/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// ChooseConnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "ChooseConnDlg.h"


// CChooseConnDlg dialog

CChooseConnDlg::CChooseConnDlg(HWND hParent)
{
	m_hWnd = NULL;
	m_hParent = hParent;
	m_nChoose = _Net_NONE;
	b1 = b2 = false;
}

CChooseConnDlg::~CChooseConnDlg()
{
}

INT_PTR CChooseConnDlg::DoModal()
{
	return DialogBoxParam(theApp.m_hInstance, MAKEINTRESOURCE(IDD), m_hParent, DialogProc, reinterpret_cast<LPARAM>(this));
}

INT_PTR CALLBACK CChooseConnDlg::DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CChooseConnDlg* pThis = nullptr;

	if (message == WM_INITDIALOG)
	{
		pThis = reinterpret_cast<CChooseConnDlg*>(lParam);
		SetWindowLongPtr(hWnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
		pThis->m_hWnd = hWnd;
	}
	else
	{
		pThis = reinterpret_cast<CChooseConnDlg*>(GetWindowLongPtr(hWnd, DWLP_USER));
	}

	if (pThis)
	{
		return pThis->HandleMessage(message, wParam, lParam);
	}

	return FALSE;
}

INT_PTR CChooseConnDlg::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		OnInitDialog();
		return TRUE;

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return TRUE;

	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			
			if (wmEvent == BN_CLICKED)
			{
				switch (wmId)
				{
				case IDOK:
					OnBnClickedOk();
					return TRUE;
				case ID_BUTTON1:
					OnBnClickedButton1();
					return TRUE;
				case IDCANCEL:
					EndDialog(m_hWnd, IDCANCEL);
					return TRUE;
				}
			}
		}
		return FALSE;

	default:
		return FALSE;
	}
}


// CChooseConnDlg message handlers

void CChooseConnDlg::SetInitialData(_NetInfo data[2])
{
	// Store the data for later use
	m_data[0] = data[0];
	m_data[1] = data[1];

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
			wchar_t buffer[256];
			swprintf_s(buffer, L"WIFI (%u.%u.%u.%u)", 
				data[0].ipv4[0], 
				data[0].ipv4[1],
				data[0].ipv4[2],
				data[0].ipv4[3]);
			s1 = buffer;

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
			wchar_t buffer[256];
			swprintf_s(buffer, L"GPRS/EDGE (%u.%u.%u.%u)", 
				data[1].ipv4[0], 
				data[1].ipv4[1],
				data[1].ipv4[2],
				data[1].ipv4[3]);
			s2 = buffer;

			b2=true;
		}
	}
}

void CChooseConnDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}

void CChooseConnDlg::OnInitDialog()
{
	// Initialize dialog controls
	SetDlgItemText(m_hWnd, IDC_RADIO1, s1.c_str());
	SetDlgItemText(m_hWnd, IDC_RADIO2, s2.c_str());
	EnableWindow(GetDlgItem(m_hWnd, IDC_RADIO1), b1 ? TRUE : FALSE);
	EnableWindow(GetDlgItem(m_hWnd, IDC_RADIO2), b2 ? TRUE : FALSE);
	
	// Select default option
	if (b1)
	{
		CheckRadioButton(m_hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
	}
	else if (b2)
	{
		CheckRadioButton(m_hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
	}
}

void CChooseConnDlg::OnSize(int cx, int cy)
{
	// Move OK button to bottom of dialog
	HWND hOkButton = GetDlgItem(m_hWnd, IDOK);
	if (hOkButton)
	{
		SetWindowPos(hOkButton, NULL, 0, cy-25, cx, 25, SWP_NOZORDER);
	}
}

void CChooseConnDlg::OnBnClickedOk()
{
	// Check which radio button is selected
	if (IsDlgButtonChecked(m_hWnd, IDC_RADIO1) == BST_CHECKED)
	{
		m_nChoose = _Net_WIFI;
	}
	else if (IsDlgButtonChecked(m_hWnd, IDC_RADIO2) == BST_CHECKED)
	{
		m_nChoose = _Net_GPRS;
	}
	else
	{
		m_nChoose = _Net_WIFI; // Default to WiFi
	}
	
	EndDialog(m_hWnd, IDOK);
}

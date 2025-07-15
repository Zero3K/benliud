/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// SelectFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "SelectFileDlg.h"


// CSelectFileDlg dialog

IMPLEMENT_DYNAMIC(CSelectFileDlg, CDialog)

CSelectFileDlg::CSelectFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectFileDlg::IDD, pParent)
{

}

CSelectFileDlg::~CSelectFileDlg()
{
}

BOOL CSelectFileDlg::IsSelected(int itemid)
{
	return m_CheckWnd.IsSelected(itemid); 
}

BOOL CSelectFileDlg::IsAnySelected()
{
	return m_CheckWnd.IsAnySelected();
}

void CSelectFileDlg::AddItems(const std::wstring& item, BOOL sel)
{
	m_StringList.push_back(item);
	m_SelectList.push_back(sel);
}

void CSelectFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSelectFileDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CSelectFileDlg message handlers

BOOL CSelectFileDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	if(!CDialog::Create(lpszTemplateName, pParentWnd)) return FALSE;


	if(!m_CheckWnd.Create(L"checkwndclassname", L"checklistwndname", WS_BORDER|WS_CHILD|WS_VSCROLL|WS_HSCROLL,
		CRect(0,0,39,40), this, IDC_CHECKLIST, NULL))
	{
		return FALSE;
	}


}

BOOL CSelectFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	if(!m_CheckWnd.SubclassWindow(GetDlgItem(IDC_CHECKLIST)->GetSafeHwnd()))
	{
		return FALSE;
	}

	for(int i=0;i<m_StringList.size();i++)
	{
		m_CheckWnd.AddItem(m_StringList[i], m_SelectList[i]);
	}

	//CRect rect;
	//this->GetClientRect(rect);
	//m_CheckWnd.MoveWindow(rect);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectFileDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	
	// TODO: Add your message handler code here
	CRect rect;
	this->GetClientRect(rect);
	rect.DeflateRect(2,2,2,2);
	m_CheckWnd.MoveWindow(rect);

	//��ʱ���Ӵ���û�н��ܵ�OnSize��Ϣ������һ����ֵ�����
	::SendMessage(m_CheckWnd.GetSafeHwnd(), 
		WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
}

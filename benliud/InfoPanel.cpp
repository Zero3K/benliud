/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// InfoPanel.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "InfoPanel.h"


// CInfoPanel

IMPLEMENT_DYNCREATE(CInfoPanel, CFormView)

CInfoPanel::CInfoPanel()
	: CFormView(CInfoPanel::IDD)
{

}

CInfoPanel::~CInfoPanel()
{
}

void CInfoPanel::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInfoPanel, CFormView)
END_MESSAGE_MAP()


// CInfoPanel diagnostics

#ifdef _DEBUG
void CInfoPanel::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CInfoPanel::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CInfoPanel message handlers

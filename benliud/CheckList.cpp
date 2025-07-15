/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// CheckList.cpp : implementation file
//

#include "stdafx.h"
#include "benliud.h"
#include "CheckList.h"


// CCheckList

IMPLEMENT_DYNCREATE(CCheckList, CScrollView)

CCheckList::CCheckList()
{
	m_nMaxStringWidth=0;
}

CCheckList::~CCheckList()
{
}


BEGIN_MESSAGE_MAP(CCheckList, CScrollView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CCheckList drawing

void CCheckList::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CCheckList::OnDraw(CDC* pDC)
{
	//CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	if(m_StringList.empty()) return;

	//CRect rect;
	//this->GetClientRect(rect);

	CPoint sp=GetScrollPosition();

	CRect trect;
	trect.left=16;
	trect.right=m_nMaxStringWidth+16;
	
	for(int i=0;i<m_StringList.size();i++)
	{
		pDC->MoveTo(0, 16*i);
		pDC->LineTo(m_nMaxStringWidth+16, 16*i);
		trect.top=i*16;
		trect.bottom=(i+1)*16;
		pDC->DrawText(m_StringList[i], &trect, DT_LEFT|DT_VCENTER);
	}

}


// CCheckList diagnostics

#ifdef _DEBUG
void CCheckList::AssertValid() const
{
	CScrollView::AssertValid();
}

#ifndef _WIN32_WCE
void CCheckList::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCheckList message handlers
void CCheckList::AddItem(CString item, BOOL sel)
{
	CDC* pDC= this->GetDC();
	CSize size=pDC->GetTextExtent(item);
	if(size.cx > m_nMaxStringWidth) {
		m_nMaxStringWidth =size.cx;
	}

	m_StringList.push_back(item);
	m_SelectList.push_back(sel);

	//SetScrollSizes(MM_TEXT, CSize(m_nMaxStringWidth+16, 16*m_SelectList.size()));

	return ;
}
BOOL CCheckList::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	this->GetClientRect(&rect);

	CBrush br(RGB(255,255,0));
	
	pDC->FillRect(rect, &br);
	return CScrollView::OnEraseBkgnd(pDC);

}

void CCheckList::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: Add your specialized code here and/or call the base class
	SetScrollSizes(MM_TEXT, CSize(m_nMaxStringWidth+16, 16*m_SelectList.size()));
}

void CCheckList::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}

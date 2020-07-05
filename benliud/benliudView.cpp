/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliudView.cpp : implementation of the CbenliudView class
//

#include "stdafx.h"
#include "benliud.h"

#include "benliudDoc.h"
#include "benliudView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CbenliudView

IMPLEMENT_DYNCREATE(CbenliudView, CListView)

BEGIN_MESSAGE_MAP(CbenliudView, CListView)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CbenliudView::OnLvnItemchanged)
END_MESSAGE_MAP()

// CbenliudView construction/destruction

CbenliudView::CbenliudView()
{
	// TODO: add construction code here

}

CbenliudView::~CbenliudView()
{
}

BOOL CbenliudView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style=cs.style|LVS_REPORT|LVS_EX_GRIDLINES;
	////cs.dwExStyle|=LVS_EX_FULLROWSELECT; 无效

	return CListView::PreCreateWindow(cs);
}



void CbenliudView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
	CString s;
	s.LoadStringW(IDS_TASKNAME);
	GetListCtrl().InsertColumn(0, s, LVCFMT_LEFT, 150);
	s.LoadStringW(IDS_PROGRESS);
	GetListCtrl().InsertColumn(1, s, LVCFMT_LEFT, 70);
	s.LoadStringW(IDS_DOWNSPEED);
	GetListCtrl().InsertColumn(2, s, LVCFMT_LEFT, 60);
	s.LoadStringW(IDS_UPSPEED);
	GetListCtrl().InsertColumn(3, s, LVCFMT_LEFT, 60);


	m_IList.Create(16, 16, ILC_COLOR, 8, 8);
	//m_IList.Add(AfxGetApp()->LoadIcon(IDI_ICON1));
	CBitmap bm;
	bm.LoadBitmapW(IDB_BITMAP1);	//wait for download
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP2);	//downloading gray
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP3);	//downloading yellow
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP4);	//downloading green
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP5);	//failed
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP6);	//checking files
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP7);	//finished
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP8);	//uploading
	m_IList.Add(&bm,RGB(255,255,255));
	bm.LoadBitmapW(IDB_BITMAP9);	//stop
	m_IList.Add(&bm,RGB(255,255,255));

	GetListCtrl().SetImageList(&m_IList, LVSIL_SMALL);


	GetListCtrl().SetExtendedStyle (
		GetListCtrl().GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER 
		);

}


// CbenliudView diagnostics

#ifdef _DEBUG
void CbenliudView::AssertValid() const
{
	CListView::AssertValid();
}

CbenliudDoc* CbenliudView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CbenliudDoc)));
	return (CbenliudDoc*)m_pDocument;
}
#endif //_DEBUG


// CbenliudView message handlers

void CbenliudView::AddNewTaskItem(int taskid, CString name)
{

	int i=GetListCtrl().InsertItem(0, name);
	if(i==-1) MessageBox(L"insert failed ");
	GetListCtrl().SetItemData(0, taskid);
	GetListCtrl().SetItemText(0, 1, L"0.0 %");
	GetListCtrl().SetItemText(0, 2, L"0.0 k/s");
	GetListCtrl().SetItemText(0, 3, L"0.0 k/s");
	//GetListCtrl(), false);
	//SetItem( RowNo , CellNo , LVIF_IMAGE , NULL , ImageListIndex, 0 , 0 , 0 );
	GetListCtrl().SetItem(0, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);

//To unhighlight an item and unfocus it
//lc.SetItemState(nItem, 0, LVIS_FOCUSED|LVIS_SELECTED);
//
//To highlight the item
//lc.SetItemState(nItem, 1, LVIS_SELECTED, LVIS_SELECTED);
//
//To unhighlight and focus it:
//lc.SetItemState(nItem, 0, LVIS_SELECTED);
//lc.SetItemState(nItem, 1, LVIS_FOCUSED, LVIS_FOCUSED);

	//GetListCtrl().SetItemState(0, 0, LVIS_FOCUSED|LVIS_SELECTED);
}

void CbenliudView::UpdateSpeed(int taskid, int upspd, int dwspd)
{

	for(int i=0;i<GetListCtrl().GetItemCount();i++)
	{
		if(GetListCtrl().GetItemData(i)==taskid)
		{
			CString s;
			s.Format(L"%0.1f k/s", float(upspd)/1024.0f);
			GetListCtrl().SetItemText(i, 3, s);
			s.Format(L"%0.1f k/s", float(dwspd)/1024.0f);
			GetListCtrl().SetItemText(i, 2, s);
			break;
		}
	}
}

void CbenliudView::UpdateProgress(int taskid, float prog)
{
	for(int i=0;i<GetListCtrl().GetItemCount();i++)
	{
		if(GetListCtrl().GetItemData(i)==taskid)
		{
			CString s;
			s.Format(L"%0.2f %%", prog*100);
			GetListCtrl().SetItemText(i, 1, s);
			break;
		}
	}
}

void CbenliudView::RemoveTask(int taskid)
{
	for(int i=0;i<GetListCtrl().GetItemCount();i++)
	{
		if(GetListCtrl().GetItemData(i)==taskid)
		{
			GetListCtrl().DeleteItem(i);
			break;
		}
	}
}

void CbenliudView::UpdateStatus(int taskid, _JOB_STATUS status, float avail)
{


	int item=-1;

	for(int i=0;i<GetListCtrl().GetItemCount();i++)
	{
		if(GetListCtrl().GetItemData(i)==taskid)
		{
			item=i;
			break;
		}
	}

	if(item==-1)
	{
		return;
	}


	switch(status)
	{
	case _JOB_NONE:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
		break;
	case _JOB_CHECKINGFILE:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 5, 0, 0, 0);
		break;
	case _JOB_RUNNING:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 1, 0, 0, 0);
		break;
	case _JOB_DOWNLOADING:
		{
			if(avail>1.0f)
			{
				GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 3, 0, 0, 0);
			}
			else if(avail>0.0f)
			{
				GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 2, 0, 0, 0);
			}
			else
			{
				GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 1, 0, 0, 0);
			}
		}
		break;
	case _JOB_UPLOADING:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 7, 0, 0, 0);
		break;
	case _JOB_FAILED:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 4, 0, 0, 0);
		break;
	case _JOB_QUIT:
		GetListCtrl().SetItem(item, 0, LVIF_IMAGE, NULL, 8, 0, 0, 0);
		break;
	default:
		break;
	}
}

void CbenliudView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	if(pNMLV->iItem >=0)
	{
		//GetListCtrl().SetItemState(0, 0, LVIS_FOCUSED|LVIS_SELECTED);
		if(pNMLV->uNewState & LVIS_FOCUSED)
		{
			DWORD taskid= GetListCtrl().GetItemData(pNMLV->iItem);
			((CMainFrame*)theApp.GetMainWnd())->SetFocusTask(taskid);
		}

	}

	*pResult = 0;
}

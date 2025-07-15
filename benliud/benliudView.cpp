/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

该代码基于GPL v2协议发布.

****************************************************************/


// benliudView.cpp : implementation of the CbenliudView class
//

#include "stdafx.h"
#include "benliud.h"

#include "benliudDoc.h"
#include "benliudView.h"
#include "MainFrm.h"

// CbenliudView construction/destruction

CbenliudView::CbenliudView()
	: m_pDocument(nullptr), m_hListView(nullptr)
{
	// TODO: add construction code here
}

CbenliudView::~CbenliudView()
{
}

BOOL CbenliudView::PreCreateWindow()
{
	// TODO: Modify the Window class or styles here
	return TRUE;
}

void CbenliudView::OnInitialUpdate()
{
	// TODO: Initialize the view
}

// CbenliudView diagnostics

#ifdef _DEBUG
void CbenliudView::AssertValid() const
{
	// Validation logic here
}

CbenliudDoc* CbenliudView::GetDocument() const // non-debug version is inline
{
	return m_pDocument;
}
#endif //_DEBUG

// CbenliudView message handlers

void CbenliudView::AddNewTaskItem(int taskid, const std::wstring& name)
{
	// TODO: Add item to ListView control
	// This will be implemented to work with the ListView in MainFrame
}

void CbenliudView::UpdateSpeed(int taskid, int upspd, int dwspd)
{
	// TODO: Update speed display for task
}

void CbenliudView::UpdateProgress(int taskid, float prog)
{
	// TODO: Update progress display for task
}

void CbenliudView::RemoveTask(int taskid)
{
	// TODO: Remove task from ListView
}

void CbenliudView::UpdateStatus(int taskid, int status, float avail)
{
	// TODO: Update status display for task
}

void CbenliudView::OnLvnItemchanged(LPNMHDR pNMHDR, LRESULT *pResult)
{
	// TODO: Handle ListView item change
	*pResult = 0;
}
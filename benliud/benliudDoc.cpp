/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliudDoc.cpp : implementation of the CbenliudDoc class
//

#include "stdafx.h"
#include "benliud.h"

#include "benliudDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CbenliudDoc

IMPLEMENT_DYNCREATE(CbenliudDoc, CDocument)

BEGIN_MESSAGE_MAP(CbenliudDoc, CDocument)
END_MESSAGE_MAP()

// CbenliudDoc construction/destruction

CbenliudDoc::CbenliudDoc()
{
	// TODO: add one-time construction code here

}

CbenliudDoc::~CbenliudDoc()
{
}

BOOL CbenliudDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CbenliudDoc serialization


void CbenliudDoc::Serialize(CArchive& ar)
{
	(ar);
}



// CbenliudDoc diagnostics

#ifdef _DEBUG
void CbenliudDoc::AssertValid() const
{
	CDocument::AssertValid();
}
#endif //_DEBUG


// CbenliudDoc commands


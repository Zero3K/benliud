/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// benliudDoc.h : interface of the CbenliudDoc class
//


#pragma once

class CbenliudDoc : public CDocument
{
protected: // create from serialization only
	CbenliudDoc();
	DECLARE_DYNCREATE(CbenliudDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();

	virtual void Serialize(CArchive& ar);


// Implementation
public:
	virtual ~CbenliudDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};



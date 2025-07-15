/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// benliudDoc.h : interface of the CbenliudDoc class
//


#pragma once

class CbenliudDoc
{
public:
	CbenliudDoc();
	virtual ~CbenliudDoc();

// Attributes
public:

// Operations
public:
	BOOL OnNewDocument();
	void Serialize();

// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const;
#endif
};



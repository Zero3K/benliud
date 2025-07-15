/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once



// CInfoPanel form view

class CInfoPanel : public CFormView
{

	friend class CMainFrame;

	DECLARE_DYNCREATE(CInfoPanel)

public:
	CInfoPanel();           // protected constructor used by dynamic creation
	virtual ~CInfoPanel();

public:
	enum { IDD = IDD_INFOPANEL };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};



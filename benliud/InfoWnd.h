/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once


// CInfoWnd frame

class CInfoWnd : public CFrameWnd
{
	friend class CMainFrame;
	DECLARE_DYNCREATE(CInfoWnd)
protected:
	CInfoWnd();           // protected constructor used by dynamic creation
	virtual ~CInfoWnd();

protected:
	DECLARE_MESSAGE_MAP()
};



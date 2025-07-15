/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#pragma once



// CInfoPanel - converted from MFC to Windows API
class CInfoPanel
{
	friend class CMainFrame;

public:
	CInfoPanel();           // protected constructor used by dynamic creation
	virtual ~CInfoPanel();

public:
	enum { IDD = IDD_INFOPANEL };
	
	// Windows API methods
	BOOL Create(HWND hParentWnd, const RECT& rect, UINT nID);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND m_hWnd;

#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

private:
	static const wchar_t* WINDOW_CLASS_NAME;
	static bool s_bClassRegistered;
	bool RegisterWindowClass();
};



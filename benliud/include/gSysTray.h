// gSysTray.h: interface for the gSysTray class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSYSTRAY_H
#define _GSYSTRAY_H

#include <wx/wx.h>
#include <wx/taskbar.h>

class gFrame;
class gSysTray : public wxTaskBarIcon 
{
public:
	gSysTray(gFrame* pmain);
	virtual ~gSysTray();
	void DlgShow( bool sh );
protected:
	void OnMenuHideRestore(wxCommandEvent& event);
	void OnMenuQuit(wxCommandEvent &event);
	void SwitchShow();
	wxMenu* CreatePopupMenu();
	void OnLeftButtonClick( wxTaskBarIconEvent& event );
	void OnRightButtonClick( wxTaskBarIconEvent& event );
	bool m_bDlgOnMain;
	gFrame *m_pMain;
private:
    DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GSYSTRAY_H__C74A421B_6FE2_4D9C_9BCF_9DE218A44927__INCLUDED_)

#pragma once
#include <wx/wx.h>
#include <wx/tipwin.h>
#include <wx/sizer.h>
#include <wx/valgen.h>
#include <wx/spinctrl.h>
#include "../include/bittorrent_def.h"
#include "../include/gTaskItemCtrl.h"

class gSpinChoice;
class gTipWinForConfig :
	public wxPopupTransientWindow
{
public:
	gTipWinForConfig(gTaskItemCtrl* parent, _BTTaskConfigInfo& info);
	~gTipWinForConfig(void);
protected:
	void OnKillFocus(wxFocusEvent& event);
	virtual void OnDismiss();
	void OnSize(wxSizeEvent& event);

	wxStaticText* t1;
	wxStaticText* t2;
	wxStaticText* t3;
	wxStaticText* t4;
	wxStaticText* t5;

	gSpinChoice* m_pCache;
	gSpinChoice* m_pUpspd;
	gSpinChoice* m_pDwspd;
	gSpinChoice* m_pMaxCon;
	gSpinChoice* m_pStopMode;

private:
	DECLARE_EVENT_TABLE()
};

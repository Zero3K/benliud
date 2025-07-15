// gTorrentTipWindow.h: interface for the gTorrentTipWindow class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTIPWINDOW_H
#define _GTIPWINDOW_H

#include <wx/wx.h>
#include <wx/minifram.h>
#include <wx/timer.h>
#include <vector>
#include <wx/statbmp.h>
#include <wx/toplevel.h>

DECLARE_EVENT_TYPE(mgEVT_HIDE_TIP, -1)
DECLARE_EVENT_TYPE(mgEVT_NEXT_TIP, -1)
DECLARE_EVENT_TYPE(mgEVT_PREV_TIP, -1)

#ifdef WIN32
#define WINWIDTH (300)
#define WINHEIGHT (110)
#else
#define WINWIDTH (300)
#define WINHEIGHT (110)
#endif

class gTipWindowLogo;
class gTipWindowHyperLink;
	
class gTipWindow 
#ifdef WIN32
	: public wxMiniFrame
#else
	: public wxTopLevelWindow
#endif
{
	struct TTip
	{
		wxString title;
		wxString url;
		wxString content;
	};

public:
	bool HavePrevTip();
	bool HaveNextTip();
	void ShowMsg(wxString title, wxString url, wxString content);
	gTipWindow(wxWindow* parent);
	virtual ~gTipWindow();

protected:
	void SetTexts();
	void OnPrev(wxCommandEvent& event);
	void OnNext(wxCommandEvent& event);
	void OnHide(wxCommandEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

	std::vector<TTip> m_MsgList;
	
	unsigned int m_nPos;

//	wxTimer m_Timer;
	unsigned int m_nLastUserActive;
	bool	m_bMouseOn;

	gTipWindowLogo	*m_pLogo;
	wxStaticText *m_pNumText;
	gTipWindowHyperLink *m_pTipTitle;
	wxTextCtrl	 *m_pTipContent;

	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GTORRENTTIPWINDOW_H__AC7742BE_2A00_4961_A2BB_F323505D02EC__INCLUDED_)

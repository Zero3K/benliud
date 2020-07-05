// gTipWindowHyperLink.h: interface for the gTipWindowHyperLink class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTIPWINDOWHYPERLINK_H
#define _GTIPWINDOWHYPERLINK_H

#include <wx/wx.h>
#include <wx/control.h>
class gTipWindowHyperLink : public wxControl 
{
public:
	void SetValues(wxString text,wxString url);
	gTipWindowHyperLink(wxWindow* parent);
	virtual ~gTipWindowHyperLink();

	DECLARE_EVENT_TABLE()
protected:
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	bool m_bMouseOn;
	wxString m_sText,m_sUrl;
};

#endif // !defined(AFX_GTIPWINDOWHYPERLINK_H__9ED8C37C_77D8_40EA_B979_C96729F6A15F__INCLUDED_)

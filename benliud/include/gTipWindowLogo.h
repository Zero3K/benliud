// gTipWindowLogo.h: interface for the gTipWindowLogo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTIPWINDOWLOGO_H
#define _GTIPWINDOWLOGO_H

#include <wx/wx.h>
#include <wx/control.h>

class gTipWindow;
class gTipWindowLogo : public wxControl 
{
public:
	gTipWindowLogo(gTipWindow* parent);
	virtual ~gTipWindowLogo();
protected:
	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GTIPWINDOWLOGO_H__B2B853A9_22BD_4894_9D46_D2C1DCF39E3B__INCLUDED_)

// gTipWindowCloseButCtrl.h: interface for the gTipWindowCloseButCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTIPWINDOWCLOSEBUTCTRL_H
#define _GTIPWINDOWCLOSEBUTCTRL_H

#include <wx/wx.h>
#include <wx/control.h>

class gTipWindow;
class gTipWindowCloseButCtrl : public wxControl
{
public:
	gTipWindowCloseButCtrl(gTipWindow* parent);
	virtual ~gTipWindowCloseButCtrl();
protected:
	void OnLeftDown(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()
private:
	bool m_bMouseOn;
//	bool m_bLeftDown;
};

#endif // !defined(AFX_GTIPWINDOWCLOSEBUTCTRL_H__B858D02D_6A83_44F7_90CC_D1D271105ABF__INCLUDED_)

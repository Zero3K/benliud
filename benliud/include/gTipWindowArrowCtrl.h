// gTipWinArrowCtrl.h: interface for the gTipWinArrowCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTIPWINDOWARROWCTRL_H
#define _GTIPWINDOWARROWCTRL_H

#include <wx/wx.h>
#include <wx/control.h>

class gTipWindow;
class gTipWindowArrowCtrl : public wxControl
{
public:
	gTipWindowArrowCtrl(gTipWindow* parent, bool leftarrow=true);
	virtual ~gTipWindowArrowCtrl();

	DECLARE_EVENT_TABLE()
protected:
	void OnLeftDown(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	bool m_bLeftArrow;
	bool m_bMouseOn;
};

#endif // !defined(AFX_GTIPWINARROWCTRL_H__EA27D68F_0024_436B_A2EF_534E5772CF47__INCLUDED_)

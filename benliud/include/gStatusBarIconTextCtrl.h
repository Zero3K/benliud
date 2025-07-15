// gStatusBarIconTextCtrl.h: interface for the gStatusBarIconTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSTATUSBARICONTEXTCTRL_H
#define _GSTATUSBARICONTEXTCTRL_H

#include <wx/wx.h>
#include <wx/control.h>
#include <wx/statbmp.h>
#include <wx/imaglist.h>

typedef enum _STATUS
{
	_OFFLINE, //gray light
	_READY, //yellow light
	_CONNECTED,	//green light
	_GOTDATA,	//flick light between yellow and green
};

class gStatusBarIconTextCtrl : public wxControl 
{
public:
	virtual void OnGetData(int val=0);
	void SetText(wxString x);
	void SetStatus(_STATUS status);
	gStatusBarIconTextCtrl(wxWindow* parent);
	virtual ~gStatusBarIconTextCtrl();
protected:
	void OnTimer(wxTimerEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent &event);
	_STATUS m_Status;
	wxString m_Text;
	wxTimer  m_flicker;
private:

	DECLARE_EVENT_TABLE()
};

#endif 
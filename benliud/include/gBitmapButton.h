#pragma once
#include <wx/wx.h>

class gBitmapButton :
	public wxWindow
{
public:
	gBitmapButton(wxWindow* parent, wxWindowID id,wxBitmap& label, wxBitmap& hover);
	~gBitmapButton(void);
	void SetBitmapLabel(wxBitmap& bitmap);
	void SetBitmapHover(wxBitmap& bitmap);

protected:
        //void OnPaint();
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvents(wxMouseEvent& event);
	wxBitmap m_Label;
	wxBitmap m_Hover;
	bool m_bMouseOn;
	DECLARE_EVENT_TABLE()
};

#pragma once

#include <wx/wx.h>
#include "gFrame.h"
class gMainTab : public wxWindow
{
public:
	gMainTab(gFrame* parent);
	~gMainTab(void);
	void SetToTorrentPage();
protected:
	void OnPaint(wxPaintEvent &event);
	void OnEraseBackground(wxEraseEvent &event);
	void OnSize(wxSizeEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	wxArrayString m_TitleList;
	wxFont m_Font;

	int m_nHotItem;
	int m_nSelectedItem;

private:
	DECLARE_EVENT_TABLE()
};

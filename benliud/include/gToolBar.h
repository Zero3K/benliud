#pragma once

#include <wx/wx.h>

DECLARE_EVENT_TYPE(ID_FINDEDIT,-1)

class gFrame;
class gToolBar : public wxToolBar
{
public:
	gToolBar(gFrame* parent);
	~gToolBar(void);
	void SetEditSize(int width);
protected:
	void OnEnter(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

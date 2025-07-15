#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/toolbook.h>

//#include <wx/aui/aui.h>

class gTaskList;
class gMainNote : public wxNotebook //wxAuiNotebook
{
public:
	gMainNote(wxWindow* parent);
	~gMainNote();

	gTaskList *m_pTab1;
private:
	DECLARE_EVENT_TABLE()
};


// gDeleteConfirm.h: interface for the gDeleteConfirm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GDELETECONFIRM_H
#define _GDELETECONFIRM_H


#include <wx/wx.h>
#include <wx/dialog.h>

DECLARE_EVENT_TYPE(mgID_DELETE_ITEM,-1)
DECLARE_EVENT_TYPE(mgID_DELETE_ALL,-1)

class gDeleteConfirm  : public wxDialog
{
public:
	bool IsOnlyDeleteItem();
	void OnDeleteAll(wxCommandEvent& event);
	void OnDeleteItem(wxCommandEvent& event);
	gDeleteConfirm(wxWindow* parent);
	virtual ~gDeleteConfirm();

	bool m_bOnlyDeleteItem;
private:
	DECLARE_EVENT_TABLE()

};

#endif 
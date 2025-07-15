#pragma once
#include <wx/wx.h>
#include <vector>
#include "../include/datatype_def.h"

class gFileListItemCtrl;
class gFileListCtrl : public wxWindow
{
public:
	gFileListCtrl(wxWindow* parent);
	~gFileListCtrl(void);
	void AddItem(wxString name, llong size, char prio='3');
	std::string GetPriority();
	llong GetSelectedSize();
	void SelectAll();
	void SelectNone();
	void SelectReverse();
protected:
	int m_nScrollPos;

	void RePaint();
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvents(wxMouseEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnScrollToTop(wxScrollWinEvent& event);
	void OnScrollToBottom(wxScrollWinEvent& event);
	void OnScrollLineUp(wxScrollWinEvent& event);
	void OnScrollLineDown(wxScrollWinEvent& event);
	void OnScrollPageUp(wxScrollWinEvent& event);
	void OnScrollPageDown(wxScrollWinEvent& event);
	void OnScrollThumbTrack(wxScrollWinEvent& event);
	void OnScrollThumbRelease(wxScrollWinEvent& event);
	void OnUpdateSelectedSize(wxCommandEvent& event);
	

	typedef std::vector<gFileListItemCtrl*>::iterator ItemItr;
	std::vector<gFileListItemCtrl*> m_Items;

	DECLARE_EVENT_TABLE()
};

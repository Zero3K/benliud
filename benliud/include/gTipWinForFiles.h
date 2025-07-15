#pragma once
#include <string>
#include <wx/wx.h>
#include <wx/tipwin.h>

class gTaskItemCtrl;
class gFileListCtrl;
class gTipWinForFiles : public wxPopupTransientWindow
{
public:
	gTipWinForFiles(gTaskItemCtrl* parent, int taskid);
	~gTipWinForFiles(void);

protected:
	void OnSelectAll(wxCommandEvent& event);

	void OnSelectNone(wxCommandEvent& event);

	void OnSelectReverse(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
	bool FileSelected(int seq, std::string& prio);
	//wxCheckListBox* m_pChoice;
	wxButton* m_pButAll;
	wxButton* m_pButNone;
	wxButton* m_pButRev;

	gFileListCtrl* m_pFileList;
	void OnDismiss();

	DECLARE_EVENT_TABLE()
};

#pragma once
#include <wx/wx.h>
#include "../include/gPathPickerCtrl.h"
#include "../include/gFileListCtrl.h"

class gFileListCtrl;
class gNewTaskDlg : public wxDialog
{
public:
	gNewTaskDlg(wxWindow* parent, std::string infohash);
	~gNewTaskDlg(void);
	wxString GetFolder(){return m_PathPicker->GetValue();}
	std::string GetPriority() {return m_pFileList->GetPriority();}
	llong GetSelectedSize() {return m_pFileList->GetSelectedSize();}
protected:
	bool FillFileItem(std::string infohash, gFileListCtrl* fdlg);
	void OnUpdateSelectedSize(wxCommandEvent& event);
	void OnUpdateDiskSpace(wxCommandEvent& event);
	llong GetDiskSpaceFromPath(wxString path);
	gPathPickerCtrl* m_PathPicker;
	gFileListCtrl* m_pFileList;
	wxStaticText* m_pSizeInfo;

	llong m_nDiskSpace;
	llong m_nSelectedSize;

	DECLARE_EVENT_TABLE()
};

// gSpeedChoiceCtrl.h: interface for the gSpeedChoiceCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSPEEDCHOICECTRL_H
#define _GSPEEDCHOICECTRL_H

#include <wx/wx.h>
#include <wx/choice.h>

class gSpeedChoiceCtrl : public wxChoice
{
public:
	void SetChoiceIndex(int index);
	int GetChoiceIndex();
	gSpeedChoiceCtrl(wxWindow* parent);
	virtual ~gSpeedChoiceCtrl();

protected:
	void OnChoice(wxCommandEvent& event);
	int m_nChoice;
	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_gSpeedChoiceCtrl_H__D687FC46_1F33_4BDD_8E8B_7973B72819F6__INCLUDED_)

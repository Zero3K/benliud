// gSpecialStopForNotFinishChoice.h: interface for the gSpecialStopForNotFinishChoice class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSPECIALSTOPFORNOTFINISHCHOICE_H
#define _GSPECIALSTOPFORNOTFINISHCHOICE_H

#include <wx/wx.h>
#include <wx/choice.h>

class gSpecialStopForNotFinishChoice : public wxChoice 
{
public:
	int GetChoiceIndex();
	void SetChoiceIndex(int index);
	gSpecialStopForNotFinishChoice(wxWindow* parent);
	virtual ~gSpecialStopForNotFinishChoice();


protected:
	void OnChoice(wxCommandEvent &event);
	int m_nChoice;
	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GSPECIALSTOPFORNOTFINISHCHOICE_H__D3CBA6E6_E0AD_4146_A527_38726C689B7D__INCLUDED_)

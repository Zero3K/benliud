// gSpecialStopForFinishedChoice.h: interface for the gSpecialStopForFinishedChoice class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSPECIALSTOPFORFINISHEDCHOICE_H
#define _GSPECIALSTOPFORFINISHEDCHOICE_H

#include <wx/wx.h>
#include <wx/choice.h>

class gSpecialStopForFinishedChoice : public wxChoice 
{
public:
	gSpecialStopForFinishedChoice(wxWindow* parent);
	virtual ~gSpecialStopForFinishedChoice();
	int GetChoiceIndex();
	void SetChoiceIndex(int index);

protected:
	void OnChoice(wxCommandEvent &event);
	int m_nChoice;
	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GSPECIALSTOPFORFINISHEDCHOICE_H__88308398_FD75_4CE3_9095_52DB1802B5B7__INCLUDED_)

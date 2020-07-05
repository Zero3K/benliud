// gDHTRunLevelChoice.h: interface for the gDHTRunLevelChoice class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GDHTRUNLEVELCHOICE_H
#define _GDHTRUNLEVELCHOICE_H

#include <wx/wx.h>
#include <wx/choice.h>

class gDHTRunLevelChoice : public wxChoice
{
public:
	gDHTRunLevelChoice(wxWindow* parent);
	virtual ~gDHTRunLevelChoice();
	int GetChoiceIndex();
	void SetChoiceIndex(int index);
protected:
	void OnChoice(wxCommandEvent &event);
	int m_nChoice;
	DECLARE_EVENT_TABLE()
};

#endif // !defined(AFX_GDHTRUNLEVELCHOICE_H__27952764_C0CF_4146_8CA5_4B5233DA18E0__INCLUDED_)

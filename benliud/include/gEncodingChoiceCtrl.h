// gEncodingChoiceCtrl.h: interface for the gEncodingChoiceCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GENCODINGCHOICECTRL_H
#define _GENCODINGCHOICECTRL_H

#include <wx/wx.h>
#include <wx/choice.h>

class gEncodingChoiceCtrl : public wxChoice
{
public:
	int GetEncodingIndex();
	void SetEncodingIndex(int idx);
	wxString GetEncoding();
	gEncodingChoiceCtrl(wxWindow* parent);
	virtual ~gEncodingChoiceCtrl();
	
protected:
	virtual void OnEncodingChanged(wxCommandEvent &event);
	int m_nEncoding;
	wxArrayString m_choices;
private:
	DECLARE_EVENT_TABLE()
};

#endif 

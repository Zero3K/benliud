#pragma once
#include <wx/wx.h>
#include <wx/spinbutt.h>

class gSpinChoice : public wxWindow
{
public:
	gSpinChoice(wxWindow* parent);
	~gSpinChoice(void);

	void SetSpinRange(int max, int offset, int step=1);
	void SetSpinValue(int spin) {m_pSpin->SetValue(spin);}

	void SetTextIntValue(int val) {m_pText->SetValue(wxString::Format(L"%d",val));}
	void SetChoiceList(wxArrayString& list);
	void SetSelectionIndex(int i); //only for choice list
	int  GetSpinValue(){return m_pSpin->GetValue();} //return only spin value 
	wxString GetTextValue() {return m_pText->GetValue();}
	long  GetTextIntValue() {
		long val;
		if(m_pText->GetValue().ToLong(&val))
		{
			return val;
		}
		else
		{
			return 0;
		}
	}

protected:
	
	wxSpinButton* m_pSpin;
	wxTextCtrl* m_pText;

	int m_nSpinOffset;
	int m_nSpinStep;

	//if m_clist is not empty, then this is choice mode
	wxArrayString m_cList;

	void OnSize(wxSizeEvent& event);
	void OnSpin(wxSpinEvent& event);
private:
	DECLARE_EVENT_TABLE()
};

#pragma once
#include <wx/wx.h>
#include "../include/datatype_def.h"
#include "../include/gSpinChoice.h"

#define ITEMHEIGHT 24

DECLARE_EVENT_TYPE(UPDATESELECTEDSIZE, -1)

class gFileListItemCtrl : public wxWindow
{
public:
	gFileListItemCtrl(wxWindow* parent, wxString name, llong size, char prio='3');
	~gFileListItemCtrl(void);

	llong GetSelectedSize() {
		if(!m_pCheck->GetValue())	{
			return 0;
		}	else {
			return m_originsize;
		}
	}

	char GetPriority() {
		if(!m_pCheck->GetValue())
		{
			return '0';
		}
		else
		{
			//switch(m_pPrio->GetSelection())
			switch(m_pPrio->GetSpinValue())
			{
			case 0:
				return '1';
			case 1:
				return '2';
			case 2:
				return '3';
			case 3:
				return '4';
			case 4:
				return '5';
			default:
				return '3';
			}
		}
	}

	bool IsChecked() {return m_pCheck->GetValue();}
	void Check() { m_pCheck->SetValue(true);}
	void UnCheck() { m_pCheck->SetValue(false);}

protected:
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnCheckBox(wxCommandEvent& event);

	wxString LengthFormat( llong length );
	//void OnEraseBackground(wxEraseEvent& event);

	//int m_nSplitter[3]; //第一，二，三垂直分割线位置


	wxCheckBox *m_pCheck;
	wxStaticText* m_pSize;
	//wxChoice* m_pPrio;
	llong m_originsize;

	gSpinChoice* m_pPrio;

	DECLARE_EVENT_TABLE()
};

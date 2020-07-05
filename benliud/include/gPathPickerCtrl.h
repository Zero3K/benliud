// gPathPickerCtrl.h: interface for the gPathPickerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GPATHPICKERCTRL_H
#define _GPATHPICKERCTRL_H

#include <wx/combo.h>

DECLARE_EVENT_TYPE(UPDATEDISKSPACE, -1)

class gPathPickerCtrl  : public wxComboCtrl  
{
public:
	gPathPickerCtrl();
	gPathPickerCtrl(
						wxWindow *parent,
                        wxWindowID id = wxID_ANY,
                        const wxString& value = wxEmptyString,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = 0,
                        const wxValidator& validator = wxDefaultValidator,
                        const wxString& name = wxComboBoxNameStr);

	virtual ~gPathPickerCtrl();
	virtual void OnButtonClick();
private:
	void Init();
};

#endif // !defined(AFX_GPATHPICKERCTRL_H__C6E3A9B8_EC4C_46D7_8C31_DD953CFCE528__INCLUDED_)

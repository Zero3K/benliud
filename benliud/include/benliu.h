//benliu.h

#ifndef _BENLIU_H
#define _BENLIU_H

#include <wx/wx.h>
#include <wx/app.h>

class CSingleCheck;
class CCmdListener;

class gApp : public wxApp
{
public:
	gApp():m_pSingleCheck(NULL), m_pCmdListener(NULL) {
	};
	virtual ~gApp() {};

	void DoCmdLine(wxString cmdline);
	void DoActive();

protected:
    virtual bool OnInit();
	virtual int  OnExit();
	void CallUp(wxArrayString args);
protected:
	CSingleCheck	*m_pSingleCheck;
	CCmdListener	*m_pCmdListener;
};

DECLARE_APP( gApp )

#endif
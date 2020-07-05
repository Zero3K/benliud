// gStatusBar.h: interface for the gStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSTATUSBAR_H
#define _GSTATUSBAR_H

#include <wx/wx.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
#include <wx/statbmp.h>
//class gStatusBarOpenKadCtrl;
class gStatusBarBittorrentDHT;
class gStatusBarExtIpCtrl;

class gStatusBar : public wxStatusBar
{
public:
	void UpnpGetData(int val);
	void UpnpGetReady(int val);
	void BtKadGetData(int val);
	void BtKadGetReady(int val);
	gStatusBar(wxWindow *parent);
	virtual ~gStatusBar();

protected:
	void OnSize(wxSizeEvent& event);
//	wxTimer m_timer;
//	gStatusBarOpenKadCtrl* m_OpenKadStatus;
	gStatusBarBittorrentDHT* m_BittorrentDHT;
	gStatusBarExtIpCtrl*	m_ExtIpShow;
private:
	DECLARE_EVENT_TABLE()
public:
	void SetUpdateTip(void);
};

#endif // !defined(AFX_GSTATUSBAR_H__2021A14D_058F_4149_8AB7_6B5771BD2E1E__INCLUDED_)

// gStatusBarBittorrentDHT.h: interface for the gStatusBarBittorrentDHT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSTATUSBARBITTORRENTDHT_H
#define _GSTATUSBARBITTORRENTDHT_H

#include "gStatusBarIconTextCtrl.h"
class gStatusBarBittorrentDHT : public gStatusBarIconTextCtrl 
{
public:
	void OnGetData(int val);
	gStatusBarBittorrentDHT(wxWindow* parent);
	virtual ~gStatusBarBittorrentDHT();
protected:
//	wxString m_OriginText;
};

#endif

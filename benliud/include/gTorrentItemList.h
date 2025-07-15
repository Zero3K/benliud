// gTorrentItemList.h: interface for the gTorrentItemList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GTORRENTITEMLIST_H
#define _GTORRENTITEMLIST_H

#include <wx/wx.h>
#include <wx/listctrl.h>

class gTorrentItemList : public wxListView
{
public:
	void SetToFileList(bool set);
	gTorrentItemList(wxWindow* parent);
	virtual ~gTorrentItemList();

protected:
	bool	m_bFileList;
};

#endif // !defined(AFX_GTORRENTITEMLIST_H__D4017FA9_1077_4886_954E_78470DE4F9FA__INCLUDED_)

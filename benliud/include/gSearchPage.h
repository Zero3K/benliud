// gSearchPage.h: interface for the gSearchPage class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSEARCHPAGE_H
#define _GSEARCHPAGE_H

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include "../include/datatype_def.h"


DECLARE_EVENT_TYPE(mgID_RECENT_UPDATE,-1)
DECLARE_EVENT_TYPE(mgID_RECENT_CREATE,-1)
DECLARE_EVENT_TYPE(mgID_RECENT_BENLIUD,-1)
DECLARE_EVENT_TYPE(mgID_PAGE_DOWN, -1)
DECLARE_EVENT_TYPE(mgID_PAGE_UP, -1)

DECLARE_EVENT_TYPE(mgID_RANGE_ALL, -1)
DECLARE_EVENT_TYPE(mgID_RANGE_BENLIUD, -1)
DECLARE_EVENT_TYPE(mgID_RANGE_NONBENLIUD, -1)

DECLARE_EVENT_TYPE(mgID_ORDERBY_CRTIME_DESC, -1)
DECLARE_EVENT_TYPE(mgID_ORDERBY_UPTIME_DESC, -1)
DECLARE_EVENT_TYPE(mgID_ORDERBY_CRTIME_ASC, -1)
DECLARE_EVENT_TYPE(mgID_ORDERBY_UPTIME_ASC, -1)

class gSearchPage : public wxPanel
{
public:
	void ResetListCtrl();
	gSearchPage(wxWindow* parent);
	virtual ~gSearchPage();
	void InitSearch(wxString str);
	void DoSearch();
	DECLARE_EVENT_TABLE()
protected:
	void InitListCtrl();
	void ExportTorrent(long item);
	void OpenTorrent(long item);
	void OnItemRightClick(wxListEvent & event);
	void InsertTorrentItem(wxString infohash, wxString mainname, int filecount, llong filesize, unsigned int date);
	void OnEnter(wxCommandEvent& event);
	void OnDoubleClickItem(wxListEvent& event);
	//void OnPageChanged(wxCommandEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnPopMenu(wxCommandEvent & event);
	void OnItemSelected(wxListEvent& event);
	void OnItemDeSelected(wxListEvent& event);

	wxString LengthFormat( llong length );
	wxListCtrl*	m_pListCtrl;

	wxTextCtrl*	m_pText;
	long		m_nRightClickItem;

	//define text range
	wxChoice*	m_pTextChoice;
	//new way
	wxChoice*	m_pTorrentRange;	//all, benliud, non-benliud
	//order
	wxChoice*	m_pTorrentOrder;


	long m_nSelectedItemId;
	//current fix search args

	int m_nTopItem;
	int m_nLimit;
	int m_nRange; //0=all, 1=benliud, 2=no benliud
	int m_nOrder; //0=by creation time(dec), 1=by update time(dec), 2=by creation time(asc), 3=by update time(asc)
	std::string m_sText; //the text for search, utf8 format
};

#endif // !defined(AFX_GSEARCHPAGE_H__D1133EB2_CC0D_48FB_899E_0BBF60203896__INCLUDED_)

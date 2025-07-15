#pragma once
#include <wx/wx.h>
#include <wx/tipwin.h>

#include "../include/bittorrent_def.h"

class gTipWinForInfo :
	public wxPopupTransientWindow
{
public:
	gTipWinForInfo(wxWindow* parent, _BtTaskDetailedInfo& info);
	~gTipWinForInfo(void);
protected:
	void OnSize(wxSizeEvent& event);

	wxStaticText* m_pName;
	wxStaticText* m_pHash;
	wxStaticText* m_pPath;
	wxStaticText* m_pFile;
	wxStaticText* m_pSize;
	wxStaticText* m_pProg;
	wxStaticText* m_pCache;
	wxStaticText* m_pSpLimit;
	wxStaticText* m_pPiece;
	wxStaticText* m_pStopMode;

	wxTextCtrl* m_pTName;
	wxTextCtrl* m_pTHash;
	wxTextCtrl* m_pTPath;
	wxTextCtrl* m_pTFile;
	wxTextCtrl* m_pTSize;
	wxTextCtrl* m_pTProg;
	wxTextCtrl* m_pTCache;
	wxTextCtrl* m_pTSpLimit;
	wxTextCtrl* m_pTPiece;
	wxTextCtrl* m_pTStopMode;

private:
	DECLARE_EVENT_TABLE()
};

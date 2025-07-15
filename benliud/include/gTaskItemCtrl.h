#pragma once

#include <wx/wx.h>
#include "bittorrent_def.h"
#include <wx/dcbuffer.h>


class gTaskList;
class gBitmapButton;
class gTaskItemCtrl : public wxWindow
{
public:
	gTaskItemCtrl(gTaskList* parent);
	~gTaskItemCtrl();
	bool Selected() {return m_bSelected;}
	void Select(bool s=true) {m_bSelected=s;}
	void RefreshProgress();
	void RefreshBitset();
	void RefreshSpeed();
	void RefreshAvialability();
	void RefreshStatus();
	void ChangeFilePriority(std::string& prio);
	void ChangeConfigration(_BTTaskConfigInfo& info);
	void SmallButtonClick(wxWindowID id);
protected:

	void DrawBorder(wxDC* dc, wxRect& rect);
	void DrawTaskName( wxDC* dc, wxRect& rect, wxString name);
	void DrawProgress( wxDC* dc, wxRect& rect, BYTE* bitset, int count, float progress);
	void DrawSpeed( wxDC* dc, wxRect& rect, _TASKSTATUS status, int usp, int dsp);
	void DrawLogo(wxDC* dc, wxRect& rect, _TASKSTATUS status, bool finished, float availability);
	void DrawButtons(wxDC* dc, wxRect& r, _TASKSTATUS status);

	void RefreshBorder();
	void OnSize( wxSizeEvent& event );
	void OnPaint( wxPaintEvent& event );
	void OnEraseBackground(wxEraseEvent &event);
	void OnMouseEvents(wxMouseEvent& event);
	float GetAverageValueInPieceData(BYTE* ds, float begin, float end, int allpieces);
	wxString GetSpeedString(int speed);

	//bool PointOnInfoButton(wxPoint& point, wxRect& r);
	//bool PointOnFileButton(wxPoint& point, wxRect& r);
	//bool PointOnConfButton(wxPoint& point, wxRect& r);
	//bool PointOnCtrlButton(wxPoint& point, wxRect& r);

	void OnFileButton(wxCommandEvent& event);
	void OnCtrlButton(wxCommandEvent& event);
	void OnInfoButton(wxCommandEvent& event);
	void OnConfButton(wxCommandEvent& event);

private:
	bool m_bSelected;

        wxFont m_Font;
#ifdef WIN32
	wxBitmapButton* m_pFileBut;
	wxBitmapButton* m_pInfoBut;
	wxBitmapButton* m_pConfBut;
	wxBitmapButton* m_pCtrlBut;
#else
        gBitmapButton* m_pInfoBut;
	gBitmapButton* m_pFileBut;
	gBitmapButton* m_pConfBut;
	gBitmapButton* m_pCtrlBut;
#endif

	gTaskList *m_pParent;
	DECLARE_EVENT_TABLE()
};

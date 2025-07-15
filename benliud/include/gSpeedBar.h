#pragma once

#include <wx/wx.h>
#include <list>

class gSpeedBar : public wxWindow
{
public:
	gSpeedBar(wxWindow* parent);
	~gSpeedBar(void);
	void PutSpeedData( int downspeed, int upspeed=0 );
protected:
    void OnPaint( wxPaintEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnTimer( wxTimerEvent& event );
	void OnEraseBackground(wxEraseEvent &event);
	wxString GetSpeedString(int speed);
private:
    int m_nMaxSpeed;
	int m_LastSecondDownload;
	int m_LastSecondUpload;
  
	wxTimer m_timer;


    std::list<int> m_DownloadSpeedList;	//instant speed
    std::list<int> m_DownloadAvgSpeedList;	//average speed
	std::list<int> m_UploadSpeedList;
	std::list<int> m_UploadAvgSpeedList;

    DECLARE_EVENT_TABLE()
};

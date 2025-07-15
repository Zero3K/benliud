#pragma once

#include <wx/wx.h>

#include <wx/aui/aui.h>

#include <vector>
//class gMainNote;
class gMainTab;
class gTaskList;
class gToolBar;
class gStatusBar;
class gSearchPage;
class gSpeedBar;

//service event
DECLARE_EVENT_TYPE(msgEVT_SERVICE_EVENT,-1)
DECLARE_EVENT_TYPE(msgEVT_MESSAGE_EVENT,-1)

//cmdline
DECLARE_EVENT_TYPE(msgEVT_UIACTIVE_EVENT,-1)
DECLARE_EVENT_TYPE(msgEVT_CMDLINE_EVENT,-1)

//bt task event
DECLARE_EVENT_TYPE(msgEVT_BTTASK_ACCEPTPEER,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_PEERINFO,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_PEER,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_DATA,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_FILEPERCENT,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_FINISHPERCENT,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_FINISHEDPIECE,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_TOTALPIECE,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_TOTAL_SELECT,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_TRACKEREVENT,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_DOWNLOADFINISH,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_FAILED,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_STOPED,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_TIMETOFINISH,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_TASKNAME,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_AVIALBILITY,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_SPEED,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_SUMBYTE,-1)


DECLARE_EVENT_TYPE(msgEVT_BTTASK_NEWTASK,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_REMOVE,-1)

//bt task status change
//DECLARE_EVENT_TYPE(msgEVT_BTTASK_RUNNING,-1)
//DECLARE_EVENT_TYPE(msgEVT_BTTASK_DELETING,-1)
//DECLARE_EVENT_TYPE(msgEVT_BTTASK_STOPING,-1)

//
//DECLARE_EVENT_TYPE(msgEVT_BTTASK_WAITING,-1)
DECLARE_EVENT_TYPE(msgEVT_BTTASK_STATUS,-1)

//ui event
DECLARE_EVENT_TYPE(msgEVT_UI_TABCHANGED, -1)
DECLARE_EVENT_TYPE(msgEVT_UI_RUNTORRENT, -1)

//search event from toolbar
DECLARE_EVENT_TYPE(msgEVT_TOOL_SEARCH, -1)

//ui


class gFrame : public wxFrame
{
public:
	gFrame(wxString wname);
	~gFrame(void);
	//task list call this
	void EnableToolBotton(bool tstart, bool tstop, bool tdelete);
#ifndef WIN32
        void TabChanged(int cat);
#endif
protected:
	void InitStatusBar();
	void InitToolBar();
	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnServiceEvent(wxCommandEvent &event);
	void OnTimer(wxTimerEvent& event);
	void OnNewTaskNotice(wxCommandEvent& event);
#ifdef WIN32
	void OnTabChanged(wxCommandEvent& event);
#endif

	void OnNewBTTask(wxCommandEvent& event );
	void OnStartTask(wxCommandEvent& event );
	void OnStopTask(wxCommandEvent& event );
	void OnDeleteTask(wxCommandEvent& event );
	void OnFinishedPieceNotice(wxCommandEvent& event);
	void OnSpeedNotice(wxCommandEvent& event);
	void OnFinishedPersentNotice(wxCommandEvent& event);
	void OnAvialbilityNotice(wxCommandEvent& event);
	void OnFinishedNotice(wxCommandEvent& event);
	void OnAcceptNotice(wxCommandEvent& event);
	void OnTaskStatusNotice(wxCommandEvent& event);
	void OnRemoveTaskNotice(wxCommandEvent& event);
	void OnVisitForum(wxCommandEvent &event);
	void OnSearch(wxCommandEvent& event);
	void OnRunTorrent(wxCommandEvent& event);
	void OnTaskData(wxCommandEvent& event);

	wxTimer 	m_Timer; //for torrent share

	gMainTab* m_pMainTab;
	gTaskList* m_pTaskList;
	gToolBar* m_pToolBar;
	gStatusBar* m_pStatusBar;
	gSearchPage* m_pSearchPage;
	gSpeedBar* m_pSpeedBar;

	std::vector<UINT> m_iplist; //to check how many benliud clients 

private:
	DECLARE_EVENT_TABLE()
};

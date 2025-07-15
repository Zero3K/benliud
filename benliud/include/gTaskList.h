#pragma once
 
#include <wx/wx.h>
#include <map>
#include <vector>
#include "datatype_def.h"
#include "bittorrent_def.h"

#define ITEM_HEIGHT (60)

enum _TASK_CATEGORY
{
	CAT_ALL,
	CAT_DOWNLOADING,
	CAT_UPLOADING,
	CAT_COMPLETE,
};

class gTaskItemCtrl;

class CTaskInfo
{
public:
	CTaskInfo():adminid(0), bitset(NULL),allbitset(NULL) {}

	int adminid;	//¹ÜÀíÆ÷ÈÎÎñID
	wxString name;	//Ãû³Æ
	wxString savefolder; 
	_TASKSTATUS status;
	bool finished;  //ÊÇ·ñÒÑÍê³É
	long pieces;	//×ÜÆ¬Êý
	llong fsize; //ÎÄŒþŽóÐ¡
	llong ssize; //Ñ¡ÔñŽóÐ¡
	BYTE *bitset;	//Íê³ÉµÄÆ¬
	BYTE *allbitset;	//ËùÓÐÓÐÐ§µÄÆ¬
	float progress; //œø¶È
	float uspeed;
	float dspeed;	//ÉÏÏÂÐÐËÙ¶È
	float availability;
};


class gFrame;
class gTaskList : public wxWindow
{
public:
	gTaskList(gFrame* parent);
	~gTaskList();

	void SwitchCat(_TASK_CATEGORY cat);
	bool GetTaskInfoForPaint(gTaskItemCtrl* client, _BtTaskInfoForListPaint& info);
	void NewTaskNotice(int tid);
	void RemoveTaskNotice(int tid);
	void PieceData(int tid, int nop, BYTE* pdata);
	void SpeedData(int tid, int dwspd, int upspd);
	void FinishedPersent(int tid, int persent);
	void Avialbity(int tid, int avial);
	void Finished(int tid);
	void LeftClickOnTaskNotice(gTaskItemCtrl* ptask, bool ctrl);
	void GetSelectedTaskID(std::vector<int>& idlist);
	void StatusChanged(int tid, _TASKSTATUS status);
	bool GetFilesInfo(gTaskItemCtrl* client, wxArrayString& files, std::string& prio);
	void ChangeFilePriority(gTaskItemCtrl* client,std::string& prio);
	void ChangeConfigration(gTaskItemCtrl* client,_BTTaskConfigInfo& info);
	void RunOrStopTask(gTaskItemCtrl* client);
	bool GetDetailedInfo(gTaskItemCtrl* client, _BtTaskDetailedInfo& info);
	int  GetTaskId(gTaskItemCtrl* client);
	bool GetConfigInfo(gTaskItemCtrl* client, _BTTaskConfigInfo& info);
protected:
	void SelectedTaskCount(int& running, int& waiting, int& stop, int& error);
	void OnEraseBackground(wxEraseEvent & event);
	void OnSize(wxSizeEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	//void OnScrollEvents(wxScrollEvent& event);
	//void OnScrollWinEvents(wxScrollWinEvent& event);
	void OnScrollToTop(wxScrollWinEvent& event);
	void OnScrollToBottom(wxScrollWinEvent& event);
	void OnScrollLineUp(wxScrollWinEvent& event);
	void OnScrollLineDown(wxScrollWinEvent& event);
	void OnScrollPageUp(wxScrollWinEvent& event);
	void OnScrollPageDown(wxScrollWinEvent& event);
	void OnScrollThumbTrack(wxScrollWinEvent& event);
	void OnScrollThumbRelease(wxScrollWinEvent& event);

	bool IsSet( unsigned int index , std::string& bitset) const;
	void UnpackBitset(std::string& bitset, int sum, BYTE* unpack);
	void RePaint();
	bool ShouldShow(_TASKSTATUS status, bool finished);
	int ShowCount();
	std::map<gTaskItemCtrl*, CTaskInfo> m_TaskItems;
	typedef std::map<gTaskItemCtrl*, CTaskInfo>::iterator TaskItr;

private:

	gFrame* m_pParent;

	int m_nScrollPos;
	_TASK_CATEGORY m_Category;

	DECLARE_EVENT_TABLE()
};

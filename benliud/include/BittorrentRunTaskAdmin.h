// BittorrentRunTaskAdmin.h: interface for the CBittorrentRunTaskAdmin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BITTORRENTRUNTASKADMIN_H
#define _BITTORRENTRUNTASKADMIN_H

#include <string>
#include <wx/wx.h>
#include <wx/string.h>
#include <vector>
#include <Mutex.h>

#include "datatype_def.h"
#include "bittorrent_def.h"
#include "msgtype_def.h"

#ifdef WIN32
#include "../include/SystemChecker.h"
#endif


class CBittorrentSingleTask;
class CBittorrentRunTaskAdmin  
{
public:
	//call back
	void PeerInfoNotice(int tid, llong arg1, void* arg3);	//ok
	void FilePercentNotice(int tid, llong fid, llong percent); //ok
	void AcceptEventNotice(int tid); //ok
	void SaveTaskPieceNotice(int tid, int nop, int* pp); //ok
	void DataNotice(int tid, int down,int up); //ok
	void DownloadFinishNotice(int tid); //ok
	void TaskFailedNotice(int tid); //ok
	void FinishedPercentNotice(int tid, int persent);	//ok
	void FinishedPieceNotice(int tid,int nop,int* pp);	//ok
	void TaskStoppedNotice(int tid);
	void TimeToFinishNotice(int tid, int ttf);
	void TaskNameNotice(int tid, wchar_t* name);
	void AvialbilityNotice(int tid, int avial, bool finished=false);
	void TotalSelectNotice(int tid, llong total,llong selected);
	void TotalPieceNotice(int tid,int nop,int* pp);
	void TrackerEventNotice(int tid, int trackerseq, int status);
	void PeerNotice(int tid,int connected,int got);
	void TaskSpeedNotice(int tid, int downspeed,int upspeed);
	void SumBytesNotice(int tid, llong down,llong up);

	//init
	void Init();
	bool GetTaskBasicInfoById(int tid, _BtTaskInfoForList& info);
	bool GetTaskBasicInfoBySeq(int seq, _BtTaskInfoForList& info);
	bool GetTaskFileInfo(int tid, wxArrayString& names, TLengthList& lens, std::string& prio);
	bool GetConfigInfoById(int tid, _BTTaskConfigInfo& info);
	//bool GetTaskBasicInfoBySeq(int seq, int& tid, wxString& name, llong& tsize, llong& ssize, bool& finished, _TASKSTATUS& status, int& piecenum, std::string& finbit);
//control
	
	int AddTask(_BtNewTask& info); //���������ţ�0=ʧ��
	void RunTask(std::vector<int>& idlist); //������
	void StopTask(std::vector<int>& idlist);
	void DeleteTask(std::vector<int>& idlist, bool withfile=false);
	void RunOrStopTask(int tid);
	//int DeleteTask(bool withfile=true);
	int DeleteTask(int tid, bool withfile=true);
	bool RunTask(int tid);
	bool StopTask(int tid, bool fordelete=false);

	bool IsNoRunning();
	void StopAllRunningTask();
	int GetRunningTaskNumber();


	void SwitchRunStatus(int tid);
	int GetTaskNumber();
	void Save();
	bool GetFileInfoById(int tid, wxArrayString& files, std::string& prio);
	bool GetDetailedInfoById(int tid, _BtTaskDetailedInfo& info);
	void ChangeFilePriority(int tid, std::string& prio);
	void ChangeConfigration(int tid, _BTTaskConfigInfo& info);
	//int GetSelectedTaskNumber();

	void AdjustFocusTaskStopMode(_BT_STOPMODE mode);
	void AdjustFocusTaskEncryptionMode(_BT_ENCMODE mode);
	void AdjustFocusTaskCacheSize(int cache);
	void AdjustFocusTaskConnLimit(int conn);
	void AdjustFocusTaskUploadLimit(int up);
	void AdjustFocusTaskDownloadLimit(int down);
	void AdjustFocusTaskFilePriority(int fileseq, int priority);
	
	//void NoticeGuiTaskStopped(int tid);
	//void NoticeGuiDeleteTaskItem(int tid);

	bool GetFullTaskInfo(int tid, 
					int& cachesize,
					int& uplimit,
					int& downlimit,
					int& piecesize,
					int& piecenum,
					_BT_ENCMODE& encmode,
					_BT_STOPMODE& stopmode,
					int& conmax,
					int& percent,
					llong& totalsize,
					llong& selectsize,
					//wxString& torrent,
					wxString& savepath,
					wxString& taskname,
					wxString& comment,
					wxString& createby,
					wxString& infohash,
					std::string& prios,
					wxArrayString& nlist,
					TLengthList& lenlist,
					wxArrayString& tlist,
					std::string& bitset);


	CBittorrentRunTaskAdmin();
	virtual ~CBittorrentRunTaskAdmin();

protected:
	void NewTaskNotice(int tid); //new
	void NoticeRemoveTask(int tid); //new

	char IntPrioToChar(int priority);
	//void NoticeGUITaskWaiting(int tid);

	void RemoveTaskFile(wxString& savepath,wxString& name,wxArrayString& nlist,wxString& idx);

	//void TaskBeginNotice(int tid);
	void NoticeTaskStatus(int tid, _TASKSTATUS status);
	//void NoticeGuiTaskStoping(int tid,bool fordelete);

	void SheduleTask();

	bool IsSelectedFile(int fileseq, std::string& prio);

private:

	bool RunBTTask(	int		taskid,
					//const wchar_t*	torrent,
					int		torsize,
					const	char*	torrent,
					int		bitsetsize,
					const	char* bitset,
					const wchar_t*	savepath,
					const	char*	prios,
					const wchar_t*	encode,
					int		uplimit,
					int		downlimit,
					_BT_ENCMODE encmode,
					_BT_STOPMODE stopmode,
					int		cachesize,
					int		connectmax);

	int 				m_TaskId;	//taskid counter

	//SockLib::CMutex		m_RunTaskListMutex;
	SockLib::CMutex		m_TaskListMutex;	//to replace run task mutex

	int					m_FocusTask;

	//std::vector<_runbtTask> m_RunTaskList;

	std::vector<_BtTaskItem> m_TaskList;	//to replace run task list

	//std::vector<int>	m_SelectList;

#ifdef WIN32
	CSystemChecker		m_SysLinkChecker;
#endif
};

#endif

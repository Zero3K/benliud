
#ifndef _BITTORRENT_DEF_H
#define _BITTORRENT_DEF_H


#include "datatype_def.h"

#include <wx/wx.h>
#include <wx/string.h>
#include <string>
#include <list>
#include <vector>
#include "bittorrent_types.h"

typedef std::vector<llong>		TLengthList;

//bt 任务管理器使用
struct _BtTaskItem
{
	int taskid;
	std::string infohash;
	_TASKSTATUS status;

	bool finished;
	int cache;		// the cache size seq
	int uplimit;		// upload speed limit seq
	int dwlimit;		// download speed limit seq
	int piecesize;
	int piecenum;
	bool dirty;		//if dirty,need write to database when quit
	int progress;	//the persent we got, don't save to db but cal by bitset when load
	_BT_ENCMODE encmode;
	_BT_STOPMODE stopmode;
	int maxconn; //the max connection

	llong totalsize;
	llong selectsize;

	wxString savepath;
	wxString taskname;
	wxString comment;
	wxString createby;
	wxString encode;
	wxString hashstring; //just for show
	
	TLengthList	  lenlist;  //full list, every file have it's length
	wxArrayString nlist;	//name list
	wxArrayString tlist;	//tracker list

	std::string bitset;
	//用一串数字来表示每个文件的优先级，数字为0到F可以表示16个级别，够用，方便
	//保存到数据库中，也方便解西，如果长度小于文件数量则后面的优先为缺省3
	//如果为空，则优先全为3
	std::string priority;
};

//用于创建新的任务而传递参数，避免过多的参数传递
struct _BtNewTask
{
	std::string infohash;
	wxString savefolder;

	int cachesize;			//用户可选
	int uplimit;			//用户可选
	int dwlimit;			//用户可选
	int connmax;			//用户可选
	_BT_ENCMODE encmode;	//用户不可选
	_BT_STOPMODE stopmode;	//用户可选
	std::string priority;	//优先级，包括文件选择在内
};


//任务列表静态数据
struct _BtTaskInfoForList
{
	int tid;
	wxString name;
	wxString savefolder;
	int piecenum;
	llong tsize;
	llong ssize;
	std::string bitset;
	float progress;
	bool finished;
	_TASKSTATUS status;
};

//bt 任务数据库保存需要的结构
struct _BtTaskInfoForDB
{
	std::string infohash;
	_TASKSTATUS status;
	bool finished;
	int cache;		// the cache size seq
	int uplimit;		// upload speed limit seq
	int dwlimit;		// download speed limit seq
	int progress;	//the persent we got, don't save to db but cal by bitset when load
	_BT_ENCMODE encmode;
	_BT_STOPMODE stopmode;
	int maxconn; //the max connection
	wxString savepath;
	wxString encode;
	std::string bitset;
	std::string priority;
};

//任务列表显示数据的传递
struct _BtTaskInfoForListPaint
{
	int tid;
	wxString name;
	wxString savefolder;
	int piecenum;
	llong tsize;
	llong ssize;
	int dwspeed;
	int upspeed;
	BYTE* bitset;
	BYTE* allbitset;
	float progress;
	float availability;
	bool finished;
	_TASKSTATUS status;
};

//数据库检索使用的结构
struct _TorrentDataItem
{
	std::string name; 
	std::string hash;
	int filecount;
	llong tsize;
	unsigned int crtime;
	
};

//用于任务信息显示, 比_BtTaskItem少一些, 也比他多一些
//比如文件数,选择的文件数,需要计算出来
struct _BtTaskDetailedInfo
{
	bool finished;
	int progress;
	int cache;		// the cache size seq
	int uplimit;		// upload speed limit seq
	int dwlimit;		// download speed limit seq
	int piecesize;
	int piecenum;
	int files;
	int selectedfiles;
	int maxconn; //the max connection

	_BT_STOPMODE stopmode;

	llong totalsize;
	llong selectsize;

	wxString savepath;
	wxString taskname;
	wxString hashstring; //just for show
};

//配置信息, 用于显示和设置
struct _BTTaskConfigInfo
{
	int cache;		// the cache size seq
	int uplimit;		// upload speed limit seq
	int dwlimit;		// download speed limit seq
	int maxconn; //the max connection
	_BT_STOPMODE stopmode;
};

#endif

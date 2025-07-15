
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

//bt ���������ʹ��
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
	//��һ����������ʾÿ���ļ������ȼ�������Ϊ0��F���Ա�ʾ16�����𣬹��ã�����
	//���浽���ݿ��У�Ҳ����������������С���ļ���������������Ϊȱʡ3
	//���Ϊ�գ�������ȫΪ3
	std::string priority;
};

//���ڴ����µ���������ݲ������������Ĳ�������
struct _BtNewTask
{
	std::string infohash;
	wxString savefolder;

	int cachesize;			//�û���ѡ
	int uplimit;			//�û���ѡ
	int dwlimit;			//�û���ѡ
	int connmax;			//�û���ѡ
	_BT_ENCMODE encmode;	//�û�����ѡ
	_BT_STOPMODE stopmode;	//�û���ѡ
	std::string priority;	//���ȼ��������ļ�ѡ������
};


//�����б�̬����
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

//bt �������ݿⱣ����Ҫ�Ľṹ
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

//�����б���ʾ���ݵĴ���
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

//���ݿ����ʹ�õĽṹ
struct _TorrentDataItem
{
	std::string name; 
	std::string hash;
	int filecount;
	llong tsize;
	unsigned int crtime;
	
};

//����������Ϣ��ʾ, ��_BtTaskItem��һЩ, Ҳ������һЩ
//�����ļ���,ѡ����ļ���,��Ҫ�������
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

//������Ϣ, ������ʾ������
struct _BTTaskConfigInfo
{
	int cache;		// the cache size seq
	int uplimit;		// upload speed limit seq
	int dwlimit;		// download speed limit seq
	int maxconn; //the max connection
	_BT_STOPMODE stopmode;
};

#endif

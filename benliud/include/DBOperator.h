// DBOperator.h: interface for the CDBOperator class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DBOPERATOR_H
#define _DBOPERATOR_H


#include <Tools.h>
#include <Mutex.h>
#include "bittorrent_def.h"

#ifdef WIN32
#include "../../thirdparty/sqlite/sqlite3.h"
#else
#include "../../thirdparty/sqlite/sqlite3.h"
#endif
#include <wchar.h>
#include <string>
#include <vector>

// a table for find finished filename
class _finishitem
{
public:
	int id;
	int cate;
	llong size;
	int ftime;
	std::string name;
	std::string location;
	std::string md5;
	std::string sha1;
	std::string baseon; //like http://www.sina.com.cn/ or torrent://hash
};

// a table for get torrent content and parse
class _torrentcont
{
public:
	std::string hash;
	std::string content;
};

// a table for search filename
class _torrentfile
{
public:
	std::string hash;
	std::string filename;
};



//a table for init the dht network collect from all the torrent
//btdht use it to initial
class _btdhtnodes
{
public:
	std::string ip;
	int port;
};

//a table for running torrent
class _runtorrent
{
public:
	std::string infohash;
	std::string savepath;
	int category;
	std::string encode;
	int encmode;
	int stopmode;
	int	cache;
	int	uplimit;
	int downlimit;
	int connmax;
	std::string priority;
	std::string bitset;
	bool finished;
	int progress;
};

//new for benliu
class _task
{
public:
	std::string infohash;
	std::string savepath;
	std::string encode;
	std::string priority;
	std::string bitset;

	bool finished;
	int encmode;
	int stopmode;
	int	cache;
	int	uplimit;
	int dwlimit;
	int connmax;
	int progress;
};

class _torrentitem
{
public:
	std::string infohash;
	std::string mainname;
	int filecount;
	llong totalsize;
	unsigned int creation;
	std::string comment;
	std::string createby;

};

class CDBOperator  
{

public:
	int SelectTorrentItem(std::string& text, int from, int limit, int range, int order, std::vector<_TorrentDataItem>& list);
	bool SaveTask(_BtTaskInfoForDB& item);
	bool RemoveTask(std::string& infohash);
	bool SelectAllTasks(std::vector<_BtTaskInfoForDB>& tasklist);
	void reorganize();
	int GetTorrentCount(int range,int textrange,std::string text);
	bool GetAllRecentBenliudTorrentHash(std::string& hashlist);
	int SelectRecentTorrentItem(int mode, int limit);
	int SelectTorrentItemByCreateBy(std::string& text);
	int SelectTorrentItemByMainName(std::string& text);
	int SelectTorrentItemByComment(std::string& text);
	bool SelectTorrentContent(std::string& hash, std::string& content);
	bool FetchTorrentItem(int seq, 
						   std::string &infohash, 
						   std::string &mainname, 
						   int &filecount, 
						   llong &totalsize,
						   unsigned int &crtime,
						   std::string &comment,
						   std::string &createby);

	int SelectTorrentItem();
	bool GetAllRecentTorrentHash(std::string& hashlist,int count=200);
	bool IsTorrentExists(std::string& infohash);

	bool UpdateRunningTorrentArgs(
		std::string& infohash,
		int	encmode,
		int	stopmode,
		int	cache,
		int	uplimit,
		int	downlimit,
		int	connmax,
		bool finished,
		std::string& priority);
	bool FetchTorrentFiles(int seq, std::string& infohash, std::string& filename);
	bool FetchRunningTorrent(int seq,
		std::string& infohash,
		std::string& savepath,
		int&	category,
		std::string& encode,
		int&	encmode,
		int&	stopmode,
		int&	cache,
		int&	uplimit,
		int&	downlimit,
		int&	connmax,
		std::string& priority,
		std::string& bitset,
		bool&	finished,
		int&	progress);

	int SelectRunningTorrent();
	bool UpdateRunningTorrentBitSet(std::string& infohash, std::string& bitset);
	bool RemoveRunningTorrent(std::string& infohash);
	bool InsertRunningTorrent(  
		std::string& infohash,
		std::string& savepath,
		int	category,
		std::string& encode,
		int	encmode,
		int	stopmode,
		int	cache,
		int	uplimit,
		int	downlimit,
		int	connmax,
		std::string& priority,
		std::string& bitset,
		bool	finished,
		int progress);

	bool SelectRunningTorrent(
		std::string& infohash,
		std::string& savepath,
		int&	category,
		std::string& encode,
		int&	encmode,
		int&	stopmode,
		llong&	totalsize,
		std::string& comment,
		int&	cache,
		int&	uplimit,
		int&	downlimit,
		int&	connmax,
		std::string& priority,
		std::string& bitset,
		bool&	finished,
		int&	progress);

	bool SelectTorrentItem(
		std::string &infohash, 
		std::string& mainname, 
		int& filecount, 
		llong& totalsize, 
		unsigned int& creationdate, 
		std::string& comment, 
		std::string& createby);

	bool InsertTorrent(std::string& infohash, 
						std::string& content , 
						std::string& mainname, 
						int filenum, 
						llong tsize, 
						unsigned int createdate, 
						std::string& comment, 
						std::string& createby);
	
	//int SelectFileByCate(int cate);
	//bool EraseFinishedFile(int key);
	//int InsertFinishedFile(int cate, wchar_t* filename, llong filesize, wchar_t* location, wchar_t* baseon, wchar_t* md5=NULL, wchar_t* sha1=NULL);
	void Stop();
	bool Start();
	CDBOperator();
	virtual ~CDBOperator();
	//std::string text2bin(std::string text);

	int SelectTorrentItem(int range,int order,int limit,int page,int textrange, std::string text);
	bool CleanTorrentByTime(unsigned int timeline);

	int m_nNewKey; //for insert

	std::vector<_finishitem> m_FinishFileQueryResult;
	std::vector<_torrentitem> m_TorrentItemQueryResult;
	std::vector<_torrentfile> m_TorrentFileQueryResult;
	std::vector<_btdhtnodes> m_BtDhtNodesQueryResult;
	std::vector<_runtorrent> m_RunningTorrentQueryResult;
protected:
#ifdef _USE_MUTEX
	CMutex m_Mutex;
#endif
	sqlite3 *m_pDB;
	//wchar_t m_savefile[512];
};

#endif

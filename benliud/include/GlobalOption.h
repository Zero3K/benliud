// GlobalOption.h: interface for the CGlobalOption class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GLOBALOPTION_H
#define _GLOBALOPTION_H

#include <wx/wx.h>
#include <wx/string.h>

#include "bittorrent_def.h"

class CSystemChecker;
class CGlobalOption  
{
public:

	int GetBtSpecialStopForFinished();
	int GetBtSpecialStopForDownloading();

	void Save();
	void Load();

	wxString Get_lastsave() {return m_LastSaveFolder;}
	void Set_lastsave(wxString path) {m_LastSaveFolder=path;}
	unsigned short Get_ts_port() {return m_ts_port;} //new
	wxString Get_ts_encode() {return m_ts_encode;}
	unsigned short Get_dht_port() {return m_dht_port;}
	unsigned int Get_dht_level() {return m_dht_level;}
	unsigned int Get_bt_maxrun() {return m_bt_maxrun;}
	unsigned int Get_global_uplimit() {return 0;}
	unsigned int Get_global_dwlimit() {return 0;}
	unsigned short Get_bt_port() {return m_bt_port;}
	unsigned int Get_bt_maxgconn() {return m_bt_maxgconn;}
	_BT_STOPMODE Get_bt_stopmode() {return (_BT_STOPMODE)m_bt_stopmode;}
	_BT_ENCMODE  Get_bt_encmode() {return (_BT_ENCMODE)m_bt_encmode;}
	unsigned int Get_bt_guspd() {return m_bt_guspd;}
	unsigned int Get_bt_gdspd() {return m_bt_gdspd;}
	unsigned int Get_bt_half_open() {return m_bt_half_open;}

	wxString GetDataFolder(); //new
	wxString GetExecFolder() {return m_ExecFolder;}
	CGlobalOption();
	virtual ~CGlobalOption();

protected:

	int m_BtSpecailStopForDownloading;
	int m_BtSpecailStopForFinished;

//新的配置
	wxString m_DataFolder;
	wxString m_ExecFolder;
	wxString m_LastSaveFolder;

	unsigned short m_bt_port;
	unsigned int m_bt_maxgconn; //全局最大连接数
	unsigned int m_bt_maxsconn; //单个任务最大连接数
	unsigned int m_bt_cache; //单个任务缓存默认
	unsigned int m_bt_dspd;	//单个任务下载限速
	unsigned int m_bt_uspd; //单个任务上传限速

	unsigned int m_bt_maxrun; //最大运行任务数
	unsigned int m_bt_gdspd;	//全局限速
	unsigned int m_bt_guspd;	//全局限速
	int m_bt_stopmode; //停止模式
	int m_bt_encmode; //加密连接选择项
	int m_bt_half_open; //半开连接限制

	unsigned short m_dht_port;
	unsigned int m_dht_level;

	unsigned short m_ts_port;
	wxString m_ts_encode;	//缺省的编码。不能识别种子编码时使用


};

#endif

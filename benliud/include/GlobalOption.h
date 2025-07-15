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

//�µ�����
	wxString m_DataFolder;
	wxString m_ExecFolder;
	wxString m_LastSaveFolder;

	unsigned short m_bt_port;
	unsigned int m_bt_maxgconn; //ȫ�����������
	unsigned int m_bt_maxsconn; //�����������������
	unsigned int m_bt_cache; //�������񻺴�Ĭ��
	unsigned int m_bt_dspd;	//����������������
	unsigned int m_bt_uspd; //���������ϴ�����

	unsigned int m_bt_maxrun; //�������������
	unsigned int m_bt_gdspd;	//ȫ������
	unsigned int m_bt_guspd;	//ȫ������
	int m_bt_stopmode; //ֹͣģʽ
	int m_bt_encmode; //��������ѡ����
	int m_bt_half_open; //�뿪��������

	unsigned short m_dht_port;
	unsigned int m_dht_level;

	unsigned short m_ts_port;
	wxString m_ts_encode;	//ȱʡ�ı��롣����ʶ�����ӱ���ʱʹ��


};

#endif

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// TCPTracker.h: interface for the CTCPTracker class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TCP_TRACKER_H
#define _TCP_TRACKER_H


#include <SockProxyTCPClientSock.h>
//#include "../../benliud/include/msgtype_def.h"
#include <Tools.h>
#include "TrackerBase.h"
#include "TTrackerDefines.h"

#include <string>
class CTrackerCenter;
class CTCPTracker : public CTrackerBase, public SockLib::CSockProxyTCPClientSock
{
public:
	void SetEvent(TTrackerEvent event);
	void Stop();

	void SetId(char* id);
	void SetHash(char* hash);

	void SetNextUpdate(int seconds);
	bool ParseHeader(std::string header, bool& red,std::string& redstr);
	void ParseContent(std::string response);
	void Update();
	void SetTracker(std::string tracker);

	virtual void OnClose();
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnConnectFail();
	virtual void OnConnectOk();
	virtual void OnTimer(unsigned int id);
	bool Start();

	CTCPTracker(CTrackerCenter* parent, int trackerseq);
	virtual ~CTCPTracker();

private:
	CTrackerCenter* m_pParent;
	std::string m_Tracker;
	std::string m_sServer;   //tracker server
	std::string m_EscPathFile;  
	std::string m_recvBuffer;
	std::string m_redirectUrl;
	unsigned short m_nPort;  //tracker port
	unsigned int m_TimerId;
	int m_Interval;
	TTrackerStatus m_Status;

	char m_Hash[20];
	char m_Id[20];

protected:

	void SendRequest(std::string server,std::string request);
	int GetBufLine( const char *buf, int start, int maxpos, char *line );
	std::string BuildTrackerUrl( std::string eventstr );
	std::string EventStr( TTrackerEvent event );
	bool SendHttpGetRequest(
    const char* shost, const char* sfile, const char* auth );
	TTrackerEvent GetCurrentEvent();
	int GetInterval();
	bool m_bRedirect;
	int	m_nFailCount;
	TTrackerEvent m_CurEvent;

	int m_nTrackerSeq;
};

#endif

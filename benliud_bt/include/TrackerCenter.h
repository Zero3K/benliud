/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// TrackerCenter.h: interface for the CTrackerCenter class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TRACKERCENTER_H
#define _TRACKERCENTER_H
#ifdef WIN32
#include <winsock2.h>
#endif
#include <ThreadBase.h>
#include "datatype_def.h"

#include <Mutex.h>
#include <AutoLock.h>

#include <string>
#include <list>
#include <Dealer.h>

//class CDealer;
class CBTStorage;
class CTrackerBase;



#include <string>
#include <list>
#include "TTrackerDefines.h"
#include "DNSBuffer.h"

typedef std::list<CTrackerBase*> TTrackerList;

class CTrackerCenter : public SockLib::CThreadBase
{

public:
	void ReleaseConnectingHandle();
	bool GetConnectingHandle();
	bool GetServerIP(std::string server, std::string &ip);
	void GetTrackerStatus(int buflen, int* status);
	void ReportTrackerStatus(int trackerseq, int status);
	void SetEvent(TTrackerEvent event);
	CBTStorage* GetStorage();
	void NoticePeerAndSeed(int total,int seed);
	bool AddPeer(unsigned int iip, unsigned short iport);
	SockLib::CDealer* GetDealer();

	void Stop();
	virtual void Entry();
	void Start();
	CTrackerCenter(CBTStorage* parent);
	virtual ~CTrackerCenter();
	//void SetBTKad(BTKADSERVICE ks);

private:
	bool m_bStop;

	SockLib::CDealer* m_pDealer;

	std::list<std::string> m_TrackerUrlList;

	CBTStorage* m_pParent;

	TTrackerList	m_TrackerList;

	TTrackerEvent	m_CurEvent;

	int*		m_pTrackerStatus;

	CDNSBuffer	m_DNSBuffer;

	SockLib::CMutex	m_ConnectingCountMutex;
	int		m_ConnectingCount;


};

#endif


// CmdListener.h: interface for the CCmdListener class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CMD_LISTENER_H
#define _CMD_LISTENER_H

#include <ThreadBase.h>

#include <wx/wx.h>
#include <wx/string.h>
#include <Dealer.h>


class CCmdListenSock;
class gApp;
class CCmdListener : public SockLib::CThreadBase 
{
public:
	void DoCmdLine(wxString cmdline);
	void DoActive();
	SockLib::CDealer* GetDealer();
	void Stop();
	bool Start();
	CCmdListener(gApp* parent);
	virtual ~CCmdListener();

protected:
	void Entry();
	bool m_bStop;
	gApp*	m_pParent;
	SockLib::CDealer* m_pDealer;
	CCmdListenSock* m_pListenSock;
};

#endif // !defined(AFX_CMDLISTENER_H__D56C2852_4C88_4DA5_902E_30332879E468__INCLUDED_)

// CmdListenSock.h: interface for the CCmdListenSock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CMD_LISTEN_SOCK_H
#define _CMD_LISTEN_SOCK_H

#include <UDPServerSock.h>
#include <BenNode.h>

class CCmdListener;
class CBenNode;
class CCmdListenSock : public SockLib::CUDPServerSock 
{
public:
	bool Start(unsigned short port);
	virtual void OnWrite();
	virtual void OnRead();
	CCmdListenSock(CCmdListener* parent);
	virtual ~CCmdListenSock();
protected:
	void DoActive();
	void DoCmdLine(BencodeLib::CBenNode& ben, sockaddr_in &addr);
	void ResponseCmdLine(sockaddr_in &addr, bool ok);
	CCmdListener* m_pParent;
};

#endif // !defined(AFX_CMDLISTENSOCK_H__B662D6A6_5A95_403B_9133_F6BECED001DC__INCLUDED_)

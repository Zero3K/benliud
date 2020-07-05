/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// UDPTracker.h: interface for the CUDPTracker class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _UDP_TRACKER_H
#define _UDP_TRACKER_H

/*
UDP tracker protocol Home  

--------------------------------------------------------------------------------

Introduction
The UDP tracker protocol is a high-performance low-overhead BitTorrent tracker protocol. URLs for this protocol look like udp://tracker:port. 

Azureus, XBT Client and XBT Tracker support this protocol. 

All values are send in network byte order (big endian). Do not expect packets to be exactly of a certain size. Future extensions could increase the size of packets. 

Set n to 0.
If no response is received after 60 * 2 ^ n seconds, resend the connect request and increase n.
If a response is received, reset n to 0.



--------------------------------------------------------------------------------

HTTP vs UDP
Ethernet: 14 bytes 
IP: 20 bytes 
TCP: 20 bytes 
UDP: 8 bytes 
Protocol Packets Non-user User Total  
HTTP 10 540 247 + 119 + 6 * N = 366 + 6 * N 906 + 6 * N  
UDP 4 168 16 + 16 + 98 + 20 + 6 * N = 150 + 6 * N 318 + 6 * N  
HTTP - UDP 6 372 216 588  
HTTP / UDP 2.5 3.2 1.5 2.0  

The UDP tracker protocol uses (less than) 50% of the bandwidth the HTTP tracker protocol uses. And because UDP is stateless, limits to the number of (open) TCP connections a router or server can handle, do not apply. 


--------------------------------------------------------------------------------

Structures
Before announcing or scraping, you have to obtain a connection ID. 

Choose a (random) transaction ID. 
Fill the connect input structure. 
Send the packet. 
connect input Offset Size Name Value  
0 64-bit integer connection_id 0x41727101980  
8 32-bit integer action 0  
12 32-bit integer transaction_id  
16  

Receive the packet. 
Check whether the packet is at least 16 bytes. 
Check whether the transaction ID is equal to the one you chose. 
Check whether the action is connect. 
Store the connection ID for future use. 
connect output Offset Size Name Value  
0 32-bit integer action 0  
4 32-bit integer transaction_id  
8 64-bit integer connection_id  
16  

Choose a (random) transaction ID. 
Fill the announce input structure. 
Send the packet. 
announce input Offset Size Name Value  
0 64-bit integer connection_id  
8 32-bit integer action 1  
12 32-bit integer transaction_id  
16 20-byte string info_hash  
36 20-byte string peer_id  
56 64-bit integer downloaded  
64 64-bit integer left  
72 64-bit integer uploaded  
80 32-bit integer event  
84 32-bit integer IP address 0  
88 32-bit integer key  
92 32-bit integer num_want -1  
96 16-bit integer port  
98  

Receive the packet. 
Check whether the packet is at least 20 bytes. 
Check whether the transaction ID is equal to the one you chose. 
Check whether the action is announce. 
Do not announce again until interval seconds have passed or an event has happened. 
announce output Offset Size Name Value  
0 32-bit integer action 1  
4 32-bit integer transaction_id  
8 32-bit integer interval  
12 32-bit integer leechers  
16 32-bit integer seeders  
20 + 6 * n 32-bit integer IP address  
24 + 6 * n 16-bit integer TCP port  
20 + 6 * N  

Up to about 74 torrents can be scraped at once. A full scrape can't be done with this protocol. 

Choose a (random) transaction ID. 
Fill the scrape input structure. 
Send the packet. 
scrape input Offset Size Name Value  
0 64-bit integer connection_id  
8 32-bit integer action 2  
12 32-bit integer transaction_id  
16 + 20 * n 20-byte string info_hash  
16 + 20 * N  

Receive the packet. 
Check whether the packet is at least 8 bytes. 
Check whether the transaction ID is equal to the one you chose. 
Check whether the action is scrape. 
scrape output Offset Size Name Value  
0 32-bit integer action 2  
4 32-bit integer transaction_id  
8 + 12 * n 32-bit integer seeders  
12 + 12 * n 32-bit integer completed  
16 + 12 * n 32-bit integer leechers  
8 + 12 * N  

If the tracker encounters an error, it might send an error packet. 

Receive the packet. 
Check whether the packet is at least 8 bytes. 
Check whether the transaction ID is equal to the one you chose. 
error output Offset Size Name Value  
0 32-bit integer action 3  
4 32-bit integer transaction_id  
8 string message  

If the tracker requires authentication, an authentication structure has to be appended to every packet you send to the tracker. The hash is the first 8 bytes of sha1(input + username + sha1(password)). authenticate input Offset Size Name  
0 8-byte zero-padded string username  
8 8-byte string hash  
16  



--------------------------------------------------------------------------------

Actions
0: connect 
1: announce 
2: scrape 
3: error 

--------------------------------------------------------------------------------

Events
0: none 
1: completed 
2: started 
3: stopped 
*/


#include <SockProxyUDPClientSock.h>
//#include "../../benliud/include/msgtype_def.h"
#include "TrackerBase.h"
#include "TTrackerDefines.h"

#include <string>
class CTrackerCenter;
class CUDPTracker : public CTrackerBase, public SockLib::CSockProxyUDPClientSock 
{
public:
	void SetEvent(TTrackerEvent event);
	void SendStopPacketWhenQuit();
	void SetHash(char* hash);
	void SetId(char* id);
	virtual void OnClose();
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnTimer(unsigned int id);

	int GetInterval();
	void SetTracker(std::string tracker);
	void SendConnectPacket();
	void Stop();
	bool Start();
	CUDPTracker(CTrackerCenter* parent, int trackerseq);
	virtual ~CUDPTracker();
	llong htonll( llong number );
//	bool parseUrl( const char* url, std::string& host, unsigned short& port, std::string& path );
	void ParseResponse( unsigned int actionID, char* data, size_t len );
	llong ntohll( llong number );
	void SendRequestPacket();


protected:
	int m_nTrackerSeq;
private:
	unsigned int m_TimerId;
	unsigned int m_nFailCount;
	SockLib::TInetAddr4 m_Server;

//	struct sockaddr_in _serverAddr;
	TTrackerStatus m_Status;
	CTrackerCenter* m_pParent;
	unsigned int m_nTransID;
	TTrackerState m_State;
	std::string m_Tracker;
	llong		m_connectionID;
//	TTrackerEvent _currentEvent;
	TTrackerEvent m_CurEvent;

	char m_Hash[20];
	char m_Id[20];
};

#endif // !defined(AFX_UDPTRACKER_H__6D87C126_F94C_446B_9E39_D5C9DFF8BC8D__INCLUDED_)

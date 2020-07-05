/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _TPEERINFO_H
#define _TPEERINFO_H

#include <string>

class CBTPeer;

struct TPeerInfo
{
    std::string		LinkId;
    std::string		Ip;
    unsigned short	Port;
    unsigned int	FailCount;
	unsigned int	LastClose;
	unsigned int	EncryptRef; //是否使用加密连接的参考值
    CBTPeer*		peerLink;
};

/*
struct TSumPeerInfo
{
    std::string LinkId;
    std::string Ip;
    unsigned short Port;
    unsigned short FailCount;	//connected fail time
	bool Seed;	//if it a seed, default is not
	unsigned int Connected;		//use bit to record which session have made up the connection
	unsigned int Connecting;	//use bit to record which session is connecting it.
	unsigned int Trys;		//how many time it give out to work
	bool Accepted;			//this is an accepted peer , don't connect it!
	bool operator < (TSumPeerInfo& other) {return FailCount < other.FailCount;};
};
*/

#endif


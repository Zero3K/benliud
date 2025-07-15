/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _ANTTASK_H
#define _ANTTASK_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <string>
#include <Sock.h> //for TInetAddr4
enum TaskType
{
	TSK_PING,
	TSK_FIND_NODE,
	TSK_GET_PEERS,
	TSK_ANNOUNCE_PEER,
	TSK_NONE,
};	
enum  AntStatus
{ 
	ANT_INIT, 
	ANT_SENDED, 
	ANT_RECEIVED, 
	ANT_TSKFINISH,
	ANT_TIMEOUT, 
	ANT_ERROR, 
};

struct TAntTask
{
	unsigned char  mykey[20];
	unsigned char  peerkey[20];	//for no response update!
	unsigned char  targetkey[20];  //hashinfo or the find_node's target node id


	unsigned short peerport;
	unsigned short dhtport; //my listen
	TaskType	   tasktype;
	std::string    peerip;	

};

//反馈包需要设置
//prio=优先级别，用于混合排队
//response=1;
//destnode=目标地址
//token=包体,全部的要反馈内容
class TRequestTask
{
public:

	unsigned short  prio;			//the task priority! the high the better
	unsigned short	lport;			//our task is listen on (for announce)

	char			peerkey[20];	//for no response update!
	char			targetkey[20];  //hashinfo or the find_node's target node id
	SockLib::TInetAddr4		destnode;		//ip and port of dest
	char			sid;			//the sequence id
	char			response;		//请求包为0，反馈包为1，这是将反馈包纳入排队的扩展
	unsigned int	outtime;		//the tick when packet send out
	//将反馈包纳入排队的扩展将这个扩展为反馈的包内容，直接发送这个内容即可，不必组织包体
	std::string		token;			//we use this token to announce ourself to peer

	TaskType		tasktype;
};

#endif

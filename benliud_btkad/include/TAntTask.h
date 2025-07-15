/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

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

//��������Ҫ����
//prio=���ȼ������ڻ���Ŷ�
//response=1;
//destnode=Ŀ���ַ
//token=����,ȫ����Ҫ��������
class TRequestTask
{
public:

	unsigned short  prio;			//the task priority! the high the better
	unsigned short	lport;			//our task is listen on (for announce)

	char			peerkey[20];	//for no response update!
	char			targetkey[20];  //hashinfo or the find_node's target node id
	SockLib::TInetAddr4		destnode;		//ip and port of dest
	char			sid;			//the sequence id
	char			response;		//�����Ϊ0��������Ϊ1�����ǽ������������Ŷӵ���չ
	unsigned int	outtime;		//the tick when packet send out
	//�������������Ŷӵ���չ�������չΪ�����İ����ݣ�ֱ�ӷ���������ݼ��ɣ�������֯����
	std::string		token;			//we use this token to announce ourself to peer

	TaskType		tasktype;
};

#endif

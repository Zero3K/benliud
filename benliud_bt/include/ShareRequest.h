/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// ShareRequest.h: interface for the CShareRequest class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SHAREREQUEST_H
#define _SHAREREQUEST_H

#include "SingleRequest.h"

class CShareRequest  
{
	class CRequestBlock
	{
	public:
		unsigned int offset;	//offset in this block
		unsigned int length;	//length of this block
		unsigned short pending;	//曾经发出过多少请求
		unsigned short comfirm;	//数据的正确性被确认的次数
		unsigned int source;	//where the data get from
		std::string  data;		//if empty ,no data
	};

	typedef std::list<CRequestBlock> TRequestList;

public:
	void ClearComfirmData();
	CShareRequest();
	virtual ~CShareRequest();
	void Init(int index, unsigned int length, int priority);
	int SetData(unsigned int iip, unsigned int offset, std::string& data);
	bool IsFinish();
	bool CheckHash(std::string& hash);
	bool GetTask( unsigned int& offset, unsigned int& length,unsigned int overlap=1);
	std::string GetPieceData();
	void SetVirtualData(unsigned int offset, unsigned int len);
	bool GetOverlapTask(unsigned int& offset, unsigned int& length);
	void SetPriority(int prio);
	void ClearData();
	int GetPriority();
	bool DataFromSameSource();
	unsigned short GetFailTimes();
	int GetIndex();
	void GiveUp(unsigned int offset);
protected:
	int Decompose(); //分解整个任务成任务片

	int	m_nIndex;	//片的索引号，-1为空
	int m_nPriority;	//片的优先级别，高优先级中的任务片可以分给多个连接去处理

	unsigned int m_nLength;	//片长
	unsigned int m_nCreator; //创建者的地址，由哪个连接首先创建这个任务
	unsigned short m_nFailTimes;
	TRequestList	m_RequestList;
};

#endif 


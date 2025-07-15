/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _SINGLEREQUEST_H
#define _SINGLEREQUEST_H

#define _CHUNK_LEN (16*1024)

#include <list>
#include <string>


class COrphan
{
public:
	int index;
	unsigned int offset;	//偏移
	unsigned int source;	//来源
	std::string data;
};

class CSingleRequest
{
	class CRequestBlock
	{
	public:
		unsigned int	offset;	//offset in this block
		unsigned int	length;	//length of this block
		unsigned int	outtick;	//the time that we send out this request
		unsigned int	source;	//数据来源
		bool			pending;	//we are waiting this chunk
		//bool			origin;	//the data is origin data from other peer			
		std::string		data;		//if empty ,no data
	};

	typedef std::list<CRequestBlock> TRequestList;

public:
	CSingleRequest(void);
	void Init(int index, unsigned int length);
	void Reset();
	bool SetData(unsigned int offset, std::string& data);
	bool IsFinish();
	bool CheckHash(std::string& hash);
	bool GetTask(unsigned int& offset, unsigned int& length);
	int GetIndex();
	void ResetPendingRequest();
	int GetPendingCount();
	void SetVirtualData(unsigned int offset, unsigned int length);
	int GetPendingRequest(int size, unsigned int offset[], unsigned int length[]);
	void SetAlien(unsigned int iip, unsigned int offset, std::string& data );
	bool HaveAlien(unsigned int iip);
	void ClearAlien(unsigned int iip);
	bool IsAllowFast();
	void SetAllowFast(bool allow=true);
	void SetCoorperate(bool coop=true) {m_bCoorperate=coop;};
	bool IsCoorperate() {return m_bCoorperate;};
	bool Empty();
	std::string GetPieceData();

	
	bool IsPendingRequest(unsigned int offset);
	//合作任务使用
	bool MarkPendingRequest(unsigned int offset);

	int TimeOutCheck(unsigned int base);
	void GetOrphans(std::list<COrphan>& orphan);

public:
	void Choke(bool choke);
	void ResetPendingRequest(unsigned int offset);
	~CSingleRequest(void);
	
protected:
	int Decompose();

	int	m_nIndex;	//片的索引号，-1为空
	unsigned int m_nLength;	//片长
	bool m_bAllowFast;
	bool m_bCoorperate;
	unsigned int m_nPending;
	//利用这两个来计算超时
	unsigned int m_nLastRequest;
	unsigned int m_nLastData;

	int m_nChoke;
	unsigned int m_nChokeTick;

	TRequestList	m_RequestList;
};


#endif


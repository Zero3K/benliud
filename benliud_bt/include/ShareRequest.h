/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

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
		unsigned short pending;	//������������������
		unsigned short comfirm;	//���ݵ���ȷ�Ա�ȷ�ϵĴ���
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
	int Decompose(); //�ֽ��������������Ƭ

	int	m_nIndex;	//Ƭ�������ţ�-1Ϊ��
	int m_nPriority;	//Ƭ�����ȼ��𣬸����ȼ��е�����Ƭ���Էָ��������ȥ����

	unsigned int m_nLength;	//Ƭ��
	unsigned int m_nCreator; //�����ߵĵ�ַ�����ĸ��������ȴ����������
	unsigned short m_nFailTimes;
	TRequestList	m_RequestList;
};

#endif 


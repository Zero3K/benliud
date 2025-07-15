/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// ShareRequest.cpp: implementation of the CShareRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/ShareRequest.h"
#include <Tools.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShareRequest::CShareRequest()
{

}

CShareRequest::~CShareRequest()
{

}


void CShareRequest::Init(int index, unsigned int length, int priority)
{
	m_RequestList.clear();
	m_nIndex=index;
	m_nLength=length;
	m_nPriority=priority;
	m_nFailTimes=0;
	Decompose();

}


int CShareRequest::GetPriority()
{
	return m_nPriority;
}

void CShareRequest::ClearData()
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(!it->data.empty())
		{
			it->data.resize(0);
			it->source=0;
		}
	}

	m_nFailTimes=0;
}


int CShareRequest::Decompose()
{
	//decompose this piece to block list m_RequestList
	unsigned int offset=0;
	while(offset< m_nLength)
	{
		CRequestBlock block;
		block.offset=offset;
		block.pending=0;
		block.source=0;
		block.comfirm=0;
		block.length= MIN(_CHUNK_LEN, m_nLength-offset);
		m_RequestList.push_back(block);		
		offset+=block.length;
	}

	return m_RequestList.size();
}


int CShareRequest::SetData(unsigned int iip, unsigned int offset, std::string& data)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset)
		{
			it->pending--;

			if(it->data.empty()) 
			{
				it->source=iip;
				it->data=data;
				return 0;
			}
			else
			{//�ظ����ݣ����

				if(it->data!=data)
				{
					if(it->comfirm == 0)
					{//û�б�ȷ�ϵ�����
						it->data=data;	//�滻�ϵ����ݣ���Ȼ���ǲ�֪���Ƿ���ȷ
						it->source=iip;	
					}

					return -2; //�����������������ȼ����ö�����������ز�ͬ����
				}
				else
				{//û�д���
					if(iip!=it->source)
					{
						it->comfirm++;
					}

					return 0;
				}
			}

		}
	}

	return -1; //not found the position
}

bool CShareRequest::IsFinish()
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->data.empty())
		{
			return false;
		}
	}

	return true;
}

bool CShareRequest::CheckHash(std::string& hash)
{
	std::string alldata;

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		alldata+=it->data;
	}

	std::string check=Tools::SHA1String(alldata);

	bool ret=check==hash;
	if(!ret) m_nFailTimes++;

	return ret;
}

//Ŀǰpenging û�л��˲�����ֻ��һ����¼���������ȥ�Ĵ���
//����ÿ��ѡ����ٷ�����Ǹ����񷵻ؼ���
bool CShareRequest::GetTask(unsigned int& offset, unsigned int& length,unsigned int overlap)
{

	//Ѱ����С��������������С������û�г������ȼ����ص��������ƣ���ѡȡ�������
	//����û�����񷵻�
	if(m_nPriority<=0) return false; //Ҳ����Ϊ���ȼ�������ȡ����������

	unsigned short least=0xFFFF; //��¼���ٵ�pending
	
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->data.empty() && it->pending < least)
		{
			least=it->pending;
			offset=it->offset;
			length=it->length;
			it->pending++;
			return true;
		}
	}

	return false; 

}

//�����������ȼ���������ָ���ص����������ṩ�ص���������
bool CShareRequest::GetOverlapTask(unsigned int& offset, unsigned int& length)
{
	if(m_nPriority <=5)	return false; //5���²������ص�����

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->data.empty() && it->pending <= 1)
		{
			offset=it->offset;
			length=it->length;
			it->pending++;
			return true;
		}
	}

	if(m_nPriority <= 10) return false; //10���ڲ�����2�������ص�����

	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->data.empty() && it->pending <= 2)
		{
			offset=it->offset;
			length=it->length;
			it->pending++;
			return true;
		}
	}

	//�ٸ�Ҳ������3�����ϵ��ص�����̫�˷Ѵ�����
	return false;
}

int CShareRequest::GetIndex()
{
	return m_nIndex;
}

void CShareRequest::SetPriority(int prio)
{
	m_nPriority=prio;
}

//����������Ҫ�����Ӽ�ס����ȡ�õ�����
void CShareRequest::GiveUp(unsigned int offset)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset) 
		{
			it->pending--;
			return;
		}
	}


}


std::string CShareRequest::GetPieceData()
{
	std::string alldata;

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		alldata+=it->data;
	}

	return alldata;
}

//�������ļ����ݣ�ȫ��0������Ӧ���Ѿ���ʼ���˲���
void CShareRequest::SetVirtualData(unsigned int offset, unsigned int len)
{
    TRequestList::iterator it;

    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->offset >= offset &&   //offset is ok
			 it->offset + it->length <= offset+len )
        {
			
            it->data.resize(it->length);
			for ( unsigned int i = 0; i < it->data.size(); i++ )
			{
				it->data[ i ] = 0;
			}

        }
    }

}

//У��ʧ��ʱ�ȿ�����������������ͬһ����ַ������������ַ
bool CShareRequest::DataFromSameSource()
{
	unsigned int iip=m_RequestList.front().source;

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->source!=iip) return false;
	}

	return true;
}

//У��ʧ�ܴ���
unsigned short CShareRequest::GetFailTimes()
{
	return m_nFailTimes;
}

void CShareRequest::ClearComfirmData()
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(!it->data.empty() && it->comfirm < m_nFailTimes )
		{
			it->data.resize(0);
			it->source=0;
		}
	}

}

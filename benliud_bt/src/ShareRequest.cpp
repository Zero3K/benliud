/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

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
			{//重复数据，检查

				if(it->data!=data)
				{
					if(it->comfirm == 0)
					{//没有被确认的数据
						it->data=data;	//替换老的数据，虽然我们不知道是否正确
						it->source=iip;	
					}

					return -2; //发生错误，最好提高优先级别让多个连接来下载不同副本
				}
				else
				{//没有错误
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

//目前penging 没有回退操作，只是一个记录曾经分配出去的次数
//所以每次选择较少分配的那个任务返回即可
bool CShareRequest::GetTask(unsigned int& offset, unsigned int& length,unsigned int overlap)
{

	//寻找最小的请求，如果这个最小的请求没有超过优先级的重叠请求限制，则选取这个任务，
	//否则没有任务返回
	if(m_nPriority<=0) return false; //也许因为优先级调整而取消掉的任务

	unsigned short least=0xFFFF; //记录最少的pending
	
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

//根据内在优先级，而不是指定重叠次数，来提供重叠请求任务
bool CShareRequest::GetOverlapTask(unsigned int& offset, unsigned int& length)
{
	if(m_nPriority <=5)	return false; //5以下不考虑重叠请求

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

	if(m_nPriority <= 10) return false; //10以内不考虑2个以上重叠请求

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

	//再高也不考虑3个以上的重叠请求，太浪费带宽了
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

//放弃任务，这要求连接记住曾经取得的任务
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

//设置虚文件数据，全是0，数据应该已经初始化了才行
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

//校验失败时先看这个，如果数据来自同一个地址，则封锁这个地址
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

//校验失败次数
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

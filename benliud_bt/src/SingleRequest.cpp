/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

#include "../include/SingleRequest.h"
#include <Tools.h>

extern void syslog(std::string info);

CSingleRequest::CSingleRequest(void)
{
	m_nIndex=-1;
	m_nPending=0; //the pending count
	m_nLastData=0;
	m_nLastRequest=0;
	m_nChoke=0;
	m_nChokeTick=GetTickCount();
}

CSingleRequest::~CSingleRequest(void)
{
}

int CSingleRequest::GetIndex()
{
	return m_nIndex;
}

void CSingleRequest::Init(int index, unsigned int length)
{
	if(index < 0)
	{
		m_nIndex=-1;
		m_nPending=0;
	}
	else
	{
		m_nIndex=index;
		m_nLength=length;
		m_bAllowFast=false;
		m_nPending=0;
		m_nLastData=0;
		m_nLastRequest=0;
		Decompose();
	}
}

int CSingleRequest::Decompose()
{
	//decompose this piece to block list m_RequestList
	unsigned int offset=0;

	while(offset< m_nLength)
	{
		CRequestBlock block;
		block.offset=offset;
		block.pending=false;
		block.outtick=0;
		block.source=0;
		block.length= _CHUNK_LEN < (m_nLength-offset) ? _CHUNK_LEN : (m_nLength-offset);
		
		offset+=block.length;

		m_RequestList.push_back(block);
	}

	return m_RequestList.size();
}

void CSingleRequest::Reset()
{
	m_RequestList.clear();
	m_nIndex=-1;
	m_nPending=0;
	m_nLastData=0;
	m_nLastRequest=0;
}

bool CSingleRequest::SetData(unsigned int offset, std::string& data)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset && it->length==data.size())
		{
			if(it->pending) 
			{
				m_nPending--;
				it->pending=false;
			}

			it->data=data;

			m_nLastData=GetTickCount();

			return true;
		}
	}

	return false; //not found the position
}

bool CSingleRequest::IsFinish()
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

bool CSingleRequest::CheckHash(std::string& hash)
{
	std::string alldata;

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		alldata+=it->data;
	}

	std::string check=Tools::SHA1String(alldata);
	return check==hash;
}

bool CSingleRequest::GetTask(unsigned int& offset, unsigned int& length)
{

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->data.empty() && !it->pending)
		{
			offset=it->offset;
			length=it->length;
			it->pending=true;
			//it->outtick=GetTickCount();
			m_nLastRequest=GetTickCount();
			m_nPending++;
			return true;
		}
	}
	return false;
}

void CSingleRequest::ResetPendingRequest()
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		it->pending=false;
	}

	m_nPending=0;
	m_nLastRequest=0;
	m_nLastData=0;
}

bool CSingleRequest::IsPendingRequest(unsigned int offset)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset) 
		{
			return (it->pending);
		}
	}

	return false;
}

bool CSingleRequest::MarkPendingRequest(unsigned int offset)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset) 
		{
			if(!it->pending)
			{
				it->pending=true;
				it->outtick=GetTickCount();

				m_nPending++;
			}
			return true;
		}
	}

	return false;
}


int CSingleRequest::GetPendingCount()
{
	return m_nPending; 
}

void CSingleRequest::SetVirtualData(unsigned int offset, unsigned int length)
{
    TRequestList::iterator it;

    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->offset >= offset &&   //offset is ok
			 it->offset + it->length <= offset+length )
        {
			
            it->data.resize(it->length);
			for ( unsigned int i = 0; i < it->data.size(); i++ )
			{
				it->data[ i ] = 0;
			}

			it->source=0; //Ҳ
        }
    }
}

int CSingleRequest::GetPendingRequest(int size, unsigned int offset[], unsigned int length[])
{
    TRequestList::iterator it;
	int counter=0;
    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->pending )
        {
			offset[counter]=it->offset;
			length[counter]=it->length;
			counter++;
			if(counter >= size) return counter;
        }
    }

	return counter;
}

//
void CSingleRequest::SetAlien(unsigned int iip, unsigned int offset, std::string& data )
{
    TRequestList::iterator it;

    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->offset == offset &&  it->length == data.size() )		
        {
            it->data = data;
            it->source = iip;
            return ;
        }
    }

}

//ͬԴ
void CSingleRequest::ClearAlien(unsigned int iip)
{
    TRequestList::iterator it;

    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->source!=iip && !it->data.empty() )		
        {
            it->data.resize(0);
            it->source = 0;	

        }
    }
}

bool CSingleRequest::HaveAlien(unsigned int iip)
{
    TRequestList::iterator it;

    for ( it = m_RequestList.begin(); it != m_RequestList.end(); it++ )
    {
        if ( it->source==iip )
        {
            return true;
        }
    }

	return false;
}

bool CSingleRequest::IsAllowFast()
{
	return m_bAllowFast;
}

void CSingleRequest::SetAllowFast(bool allow)
{
	m_bAllowFast=allow;
}

bool CSingleRequest::Empty()
{
	return  m_nIndex==-1 || m_nIndex < 0;
}

std::string CSingleRequest::GetPieceData()
{
	std::string result;

	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		result+=it->data;
	}

	return result;
}

//now = tick count
//base = tick gap
//ҵһʱ㳬ʱһ㶼ǰ˳򷢰
int CSingleRequest::TimeOutCheck(unsigned int base)
{
	if(m_nChoke % 2 ==0)
	{//be choked!
		if(m_nChoke > 10) return -1; //two much choke and unchoke in a task
		else if( GetTickCount() - m_nChokeTick > base) return -2; //too long choke
		else return 0;
	}

	if(m_nLastRequest==0) return 0; //no request

	if(m_nLastData==0) 
	{
		if(GetTickCount() - m_nLastRequest > base) return -3;
		else return 0;
	}

	if(GetTickCount() - m_nLastData > base) return -4;
	return 0;

}

//source ⲿ
void CSingleRequest::GetOrphans(std::list<COrphan>& orphan)
{
	TRequestList::iterator it;

	if(m_RequestList.empty()) return;

	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(!it->data.empty())
		{
			COrphan item;
			item.data=it->data;
			item.index=m_nIndex;
			item.offset=it->offset;
			item.source=it->source;
			orphan.push_back(item);
		}
	}

}

void CSingleRequest::ResetPendingRequest(unsigned int offset)
{
	TRequestList::iterator it;
	for(it=m_RequestList.begin();it!=m_RequestList.end();it++)
	{
		if(it->offset==offset)
		{
			if(it->pending)
			{
				it->pending=false;
				m_nPending--;
			}
			break;
		}
	}

}

//for fast externtion client
//the request won't resend when unchoked, so hard to judge timeout with out knowing
//choke status
void CSingleRequest::Choke(bool choke)
{
	m_nChoke++;

	if(choke)
	{
		if(m_nChoke%2)	m_nChoke++;

		m_nChokeTick=GetTickCount();
		m_nLastData=0;
	}

	if(!choke)
	{
		if(m_nChoke%2==0) m_nChoke++;
		//because fast externsion don't send request again
		//if don't set it , may timeout very soon after unchoke
		m_nLastRequest=GetTickCount(); 

	}
}

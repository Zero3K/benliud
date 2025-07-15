
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#include "stdafx.h"

#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "../include/SpeedControl.h"
#include "../include/BTPeer.h"
#include "../include/BTSession.h"
#include "../include/BTStorage.h"
#include <Tools.h>
#include <AutoLock.h>

#include <assert.h>


extern void syslog(std::string info);

using namespace std;

CSpeedControl::CSpeedControl( CBTSession* parent )
{
    m_pParent = parent;
}

CSpeedControl::~CSpeedControl()
{
}

void CSpeedControl::RegisteClient( CBTPeer* client )
{
	//ÓÉŒàÌý×ªÒÆ¹ýÀŽÊ±£¬ÊÇ¿çÏß³ÌµÄ£¬ËùÒÔ²»ÄÜÖ±œÓŒÓµœÔËÐÐÁÐ±íÉÏ
	//ÒªÀàËÆÓÚŒžÊ±Æ÷ÄÇÑù

	SockLib::CAutoLock al(m_PendingListMutex);
	m_PendingList.push_back(client);

}

void CSpeedControl::UnregisteClient( CBTPeer* client )
{
    //make remove mark, remove in update
	SockLib::CAutoLock al(m_PendingListMutex);

    TClientList::iterator it;
//ÕâÀïÓÐÎÊÌâ, ²»œâ!!!

    for (  it = m_ClientList.begin(); it != m_ClientList.end(); it++ )
    {
        if ( ( *it ) == client )
        {
            ( *it ) = NULL;
            break;
        }
    }

//ÐÂµÄclientŒÓÈëœøÀŽ

    for ( it = m_PendingList.begin(); it != m_PendingList.end(); ++it )
    {
		if( (*it) == client) {
			m_PendingList.erase(it);
			break;
		}
    }

}


//do upload job
void CSpeedControl::Upload()
{
//ÊÇ·ñÐèÒª°ŽÉÏŽ«ÓÅÏÈŒ¶ÅÅÐòºóÔÙ×÷£¿

	//int left= m_pParent->GetStorage()->GetLeftUpBytes();
	//if(left <=0)
	//{
	//	OutputDebugString(L"do upload no left up bytes!!!\n");
	//	return;
	//}

	float upratio=
		float(m_pParent->GetStorage()->GetSumOfUpload())/float(m_pParent->GetStorage()->GetSumOfDownload());

	//CAutoLock al(m_PendingListMutex);

	int left=1024;

    TClientList::iterator it;

	//command send
    for ( it = m_ClientList.begin(); it != m_ClientList.end();it++ )
	{

        if ( (*it)!=NULL )
        {
			
			int nret = (*it)->DoCmdWrite(left);
			left = m_pParent->GetStorage()->RunOffUpBytes(nret);

			if(left <= 0) {
				//OutputDebugString(L"no bytes to upload-1\n");
				return;
			}

        }
    }

	//balence piece block send 
    for ( it = m_ClientList.begin(); it != m_ClientList.end();it++ )
    {
        if ( (*it)!=NULL )
        {
			int nret = (*it)->DoDataWrite(left);
			left = m_pParent->GetStorage()->RunOffUpBytes(nret);

			if(left <= 0) {
				//OutputDebugString(L"no bytes to upload-2\n");
				return;
			}

        }
    }


	//equal send if we have extra bandwidth
    for ( it = m_ClientList.begin(); it != m_ClientList.end();it++ )
    {
        if ( (*it)!=NULL )
        {
			int nret = (*it)->DoEqualWriteForDownloadMode(left, upratio < 0.25f);
			left = m_pParent->GetStorage()->RunOffUpBytes(nret);

			if(left <= 0) {
				//OutputDebugString(L"no bytes to upload-3\n");
				return;
			}

        }
    }


}

//do download job
void CSpeedControl::Download()
{
	int left=m_pParent->GetStorage()->GetLeftDownBytes();

	if(left <=0)
	{
		return;
	}

	//CAutoLock al(m_PendingListMutex);
    TClientList::iterator it ;

    for ( it = m_ClientList.begin(); it != m_ClientList.end(); it++ )
    {

        if ( (*it)!=NULL )
        {

			int ret= (*it)->DoRead(left); //_downCount can <=0

			left=m_pParent->GetStorage()->RunOffDownBytes(ret);
			
			if(left<=0) return;

        }

    }


}

void CSpeedControl::Update()
{


  	CleanClient();
	
	if(m_pParent->GetStorage()->GetLeftDownBytes() <=0 )
	{

		Sleep(10);

	}
	else
	{
//		OutputDebugString(L"have dw bytes, do download\n");
		Download();
	}

	
	if(m_pParent->GetStorage()->GetLeftUpBytes() <=0)
	{

		Sleep(10);

	}
	else
	{
//		OutputDebugString(L"have up bytes, do upload\n");
		Upload();
	}
	
//for balence , move the head to the tail
	if(!m_ClientList.empty())
	{
		CBTPeer* head=m_ClientList.front();
		m_ClientList.pop_front();
		m_ClientList.push_back(head);
	}

}


void CSpeedControl::CleanClient()
{

	SockLib::CAutoLock al(m_PendingListMutex);

	TClientList::iterator it;

    for ( it = m_ClientList.begin(); it != m_ClientList.end(); )
    {
        if ( *it == NULL )
			m_ClientList.erase(it++); 
        else
            it++;
    }

//ÐÂµÄclientŒÓÈëœøÀŽ

    for ( it = m_PendingList.begin(); it != m_PendingList.end(); it++ )
    {
		m_ClientList.push_back( *it );
    }

    m_PendingList.clear();


}

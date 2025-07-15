/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#if defined(WINCE)
#include <windows.h>
#elif defined(WIN32)
#include <winsock2.h>
#include <errno.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#endif


#include "../include/UPnpNat.h"
#include "../include/UPnpThread.h"
#include <Dealer.h>

extern UINT	gStateCode;	//系统状态码
/***************************************************************
** General Defines                                             *
****************************************************************/

#define DELETE_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n" \
                                   "<NewExternalPort>%i"                 \
                                   "</NewExternalPort>\r\n"              \
                                   "<NewProtocol>%s</NewProtocol>\r\n"



CUPnpNat::CUPnpNat(CUPnpThread* parent)
{
    m_pDealer = NULL;
    _state = NS_INIT;
    m_discoverTimerID = 0;
    _controlTimerID = 0;
	m_nCheckIpTimer=0;
	m_pParent=parent;
	m_nFailTime=0;
	
}

CUPnpNat::~CUPnpNat()
{

    //TNatTaskList::iterator it;

    //for ( it=m_TaskList.begin(); it != m_TaskList.end(); it )
    //{
    //    if ( it->controller != NULL )
    //    {
    //        //it->controller->stop();
    //        delete it->controller;
    //    }
    //}

}

void CUPnpNat::SetDealer( SockLib::CDealer* dealer )
{
    m_pDealer = dealer;
}


void CUPnpNat::Start()
{
    _state = NS_DISCOVER;

    _natExplorer.SetDealer( m_pDealer );
    _natExplorer.setEventHandler( this );
    _natExplorer.Start();

	OutputDebugString(L"searching upnp device ...");

	gStateCode=30;

    if ( m_discoverTimerID != 0 )
    {
        m_pDealer->RemoveTimer( m_discoverTimerID );
    }

    m_discoverTimerID = m_pDealer->AddTimer( this, 15000*(m_nFailTime+1) , true); 

    if ( _controlTimerID != 0 )
    {
        m_pDealer->RemoveTimer( _controlTimerID );
    }

    _controlTimerID = m_pDealer->AddTimer( this, 1000 );

	if ( m_nCheckIpTimer!=0)
	{
        m_pDealer->RemoveTimer( m_nCheckIpTimer );
	}

	m_nCheckIpTimer=m_pDealer->AddTimer( this, 120000 ); //2 min check ip one time

}

void CUPnpNat::Stop()
{
    if ( m_discoverTimerID != 0 )
    {
        m_pDealer->RemoveTimer( m_discoverTimerID );
        m_discoverTimerID = 0;
    }

    if ( _controlTimerID != 0 )
    {
        m_pDealer->RemoveTimer( _controlTimerID );
        _controlTimerID = 0;
    }

	if ( m_nCheckIpTimer!=0)
	{
        m_pDealer->RemoveTimer( m_nCheckIpTimer );
		m_nCheckIpTimer=0;
	}

    _natExplorer.Stop();
    _natParser.Stop();

    TNatTaskList::iterator it;

    for ( it=m_TaskList.begin(); it != m_TaskList.end(); it++ )
    {
        if ( it->controller != NULL )
        {
            //it->controller->stop();

            delete it->controller;

        }
    }
}

TUPnpNATState CUPnpNat::getState()
{
    return _state;
}

void CUPnpNat::addPortMapping( unsigned int port, std::string protocol )
{
    TNatTaskInfo info;
    info.port = port;
    info.protocol = protocol;
	info.action = AC_ADDPORT;
	info.retry=0;

    if ( _state != NS_OK )
    {
        info.controller = NULL;
    }
    else
    {

        info.controller = new CUPnpNatController();
        info.controller->SetDealer( m_pDealer );

        info.controller->SetParent( this );
        info.controller->setControlUrl( m_sControlUrl.c_str(), _service.c_str() );
        info.controller->setAction( info.action, port, protocol );
        info.controller->start();
    }

    m_TaskList.push_back( info );
}

void CUPnpNat::removePortMapping( unsigned int port, std::string protocol )
{
    TNatTaskInfo info;
    info.port = port;
    info.protocol = protocol;
	info.action = AC_DELPORT;
	info.retry=0;

    if ( _state != NS_OK )
    {
        info.controller = NULL;
        
    }
    else
    {
        info.controller = new CUPnpNatController();
        info.controller->SetDealer( m_pDealer );

        info.controller->SetParent( this );
        info.controller->setControlUrl( m_sControlUrl.c_str(), _service.c_str() );
        info.controller->setAction( info.action, port, protocol );
        info.controller->start();
    }

    m_TaskList.push_back( info );
}

void CUPnpNat::OnTimer( unsigned int id )
{

    if ( id == m_discoverTimerID )
    {
		m_discoverTimerID=0;

        if ( _state != NS_OK )
        {
            _natExplorer.Stop();
			m_nFailTime++;
            Start();
        }

    }

    if ( id == _controlTimerID )
    {

		if ( _state != NS_OK ) return;


		TNatTaskList::iterator it;
		
		//if have del port operator ,do it first until no del operator
		bool onlydel=false;
		for ( it = m_TaskList.begin() ; it != m_TaskList.end(); ++it )
		{
			if ( it->controller == NULL && it->action==AC_DELPORT)
			{
				onlydel=true;
				break;
			}
		}

		if(!onlydel)
		{
			for ( it = m_TaskList.begin() ; it != m_TaskList.end(); ++it )
			{
				if ( it->controller == NULL)
				{
					
					it->controller = new CUPnpNatController();
					it->controller->SetDealer( m_pDealer );

					
					it->controller->SetParent( this );
					it->controller->setControlUrl( m_sControlUrl.c_str(), _service.c_str() );
					it->controller->setAction( it->action, it->port, it->protocol.c_str() );
					it->controller->start();
					
					break; //only one request in a circle!!!
				}
			}
		}
		else
		{//only del this circle
			for ( it = m_TaskList.begin() ; it != m_TaskList.end(); ++it )
			{
				if ( it->controller == NULL && it->action==AC_DELPORT)
				{
					
					it->controller = new CUPnpNatController();
					it->controller->SetDealer( m_pDealer );
					
					it->controller->SetParent( this );
					it->controller->setControlUrl( m_sControlUrl.c_str(), _service.c_str() );
					it->controller->setAction( it->action, it->port, it->protocol.c_str() );
					it->controller->start();
					
					break; //only one request in a circle;
				}
			}
		}

    }


	if(m_nCheckIpTimer==id)
	{//time to check eip
		GetExternIp();
	}
}

void CUPnpNat::OnGetDescriptionUrl( std::string&  url )
{

    if ( _state == NS_GETDESCRIPTION )
    {
        return ;
    }

    _state = NS_GETDESCRIPTION;

	m_sDescriptionUrl=url;  //save the description url to use

	_natExplorer.Stop(); //stop to find the nat


	OutputDebugString(L"found an upnp device");
	

    _natParser.Stop();
    _natParser.setEventHandler( this );
    _natParser.setDescriptionUrl( m_sDescriptionUrl );
    _natParser.SetDealer( m_pDealer );
    _natParser.Start(); //try to get control url

}

void CUPnpNat::OnGetControlUrl( const char* controlUrl, const char* service )
{
    m_sControlUrl = controlUrl;
    _service = service;
    _state = NS_OK;

	gStateCode=50;

	OutputDebugString(L"got the control url");
}

void CUPnpNat::GetExternIp()
{
    TNatTaskInfo info;
    info.action = AC_GETIP;
	info.retry=0;

    if ( _state != NS_OK )
    {
        info.controller = NULL;
    }
    else
    {
        info.controller = new CUPnpNatController();
        info.controller->SetDealer( m_pDealer );

        info.controller->SetParent( this );
        info.controller->setControlUrl( m_sControlUrl.c_str(), _service.c_str() );
        info.controller->setAction( info.action ,0, "");
        info.controller->start();
    }

    m_TaskList.push_back( info );
}

//call by controller
void CUPnpNat::NoticeExternIP( bool ok, std::string ip,int ncode )
{

	if(ok)
	{

		if(m_sExternIP!=ip) {

			m_sExternIP=ip;
			m_pParent->NoticeExternIP(ip);

			gStateCode=60;

			OutputDebugString(L"got extern ip address");
		}

		
		//delete the getip task
		TNatTaskList::iterator it;
		
		for ( it = m_TaskList.begin() ; it != m_TaskList.end(); it++)
		{
			if ( it->action==AC_GETIP && it->controller!=NULL)
			{

				delete it->controller;

				m_TaskList.erase(it);

				break;
			}
		}
	}
	else
	{

		OutputDebugString(L"To get extern ip failed");

		TNatTaskList::iterator it;
		
		//if have del port operator ,do it first until no del operator
		for ( it = m_TaskList.begin() ; it != m_TaskList.end();it++)
		{
			if ( it->action==AC_GETIP && it->controller!=NULL)
			{
				if(it->retry > 5)
				{
				
					gStateCode=61;
					delete it->controller;
					m_TaskList.erase(it);

					OutputDebugString(L"remove the task to get extern ip");
			
				}
				else
				{
			
					gStateCode=62;
					delete it->controller;
					it->controller=NULL;  //set to NULL will start again.
					it->retry++;
			
				}

				break;
			}
		}

	}
}

void CUPnpNat::NoticePortMapping( bool ok, bool addport, int inport, std::string protocol, int ncode)
{


	if(ok)
	{

		TNatTaskList::iterator it;
		
		//if have del port operator ,do it first until no del operator
		for ( it = m_TaskList.begin() ; it != m_TaskList.end(); it++)
		{
			if ( it->action==(addport?AC_ADDPORT:AC_DELPORT) && it->controller!=NULL && it->port==inport && it->protocol==protocol)
			{
			
				delete it->controller;
				//it->controller=NULL;  //set to NULL will start again.
				m_TaskList.erase(it);
				break;
			}
		}
		
		OutputDebugString(L"add/del port map ok");

	}
	else
	{
		
		if(addport)
		{

			TNatTaskList::iterator it;

			//if have del port operator ,do it first until no del operator
			for ( it = m_TaskList.begin() ; it != m_TaskList.end(); it++)
			{
				if ( it->action==AC_ADDPORT && it->controller!=NULL && it->port==inport && it->protocol==protocol)
				{
					if(it->retry > 5)
					{
					
						delete it->controller;
						//it->controller=NULL;  //set to NULL will start again.
						m_TaskList.erase(it);
					
					}
					else
					{
					
						delete it->controller;
						it->controller=NULL;  //set to NULL will start again.
						it->retry++;
					
					}
					break;
				}
			}

			//add a remove port for maybe the port is opened by other
			removePortMapping(inport,protocol);

		}
		else
		{

			TNatTaskList::iterator it;
			
			//if have del port operator ,do it first until no del operator
			for ( it = m_TaskList.begin() ; it != m_TaskList.end(); it++)
			{
				if ( it->action==AC_DELPORT && it->controller!=NULL && it->port==inport && it->protocol==protocol)
				{
				
					delete it->controller;
					//it->controller=NULL;  //set to NULL will start again.
					m_TaskList.erase(it);
				
					break;
				}
			}
		}
		
	}
}


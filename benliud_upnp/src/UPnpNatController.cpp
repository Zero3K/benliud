/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#include "../include/UPnpNatController.h"
//#include "../include/utils.h"


#if defined(WINCE)
#include <windows.h>
#elif defined(WIN32)
#include <winsock2.h>
#include <errno.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif


#include <Dealer.h>

#include "../../thirdparty/tinyxml/tinyxml.h"
#include <Tools.h>
#include "../include/UPnpNat.h"

//const char* CUPnpNatController::ActionAdd = "AddPortMapping";
//const char* CUPnpNatController::ActionDelete = "DeletePortMapping";

#define HTTP_OK "200 OK"
#define HTTP_HEADER_ACTION "POST %s HTTP/1.1\r\n"                         \
                           "HOST: %s\r\n"                                  \
                           "SOAPACTION: "                                  \
                           "\"urn:schemas-upnp-org:"                       \
                           "service:%s#%s\"\r\n"                           \
                           "CONTENT-TYPE: text/xml ; charset=\"utf-8\"\r\n"\
						   "Content-Length: %u\r\n\r\n"

#define SOAP_ACTION  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"     \
                     "<s:Envelope xmlns:s="                               \
                     "\"http://schemas.xmlsoap.org/soap/envelope/\" "     \
                     "s:encodingStyle="                                   \
                     "\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n" \
                     "<s:Body>\r\n"                                       \
                     "<u:%s xmlns:u="                                     \
                     "\"urn:schemas-upnp-org:service:%s\">\r\n%s"         \
                     "</u:%s>\r\n"                                        \
                     "</s:Body>\r\n"                                      \
                     "</s:Envelope>\r\n"

//#define PORT_MAPPING_LEASE_TIME "0"
#define PORT_MAPPING_DESCRIPTION "Benliud client %d/%s"

#define ADD_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n"      \
                                "<NewExternalPort>%i</NewExternalPort>\r\n"\
                                "<NewProtocol>%s</NewProtocol>\r\n"        \
                                "<NewInternalPort>%i</NewInternalPort>\r\n"\
                                "<NewInternalClient>%s"                    \
                                "</NewInternalClient>\r\n"                 \
                                "<NewEnabled>1</NewEnabled>\r\n"           \
                                "<NewPortMappingDescription>"              \
                                PORT_MAPPING_DESCRIPTION				\
                                "</NewPortMappingDescription>\r\n"         \
                                "<NewLeaseDuration>0</NewLeaseDuration>\r\n"

#define DELETE_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n" \
                                   "<NewExternalPort>%i"                 \
                                   "</NewExternalPort>\r\n"              \
                                   "<NewProtocol>%s</NewProtocol>\r\n"


/*
- <action>
  <name>GetExternalIPAddress</name> 
- <argumentList>
- <argument>
  <name>NewExternalIPAddress</name> 
  <direction>out</direction> 
  <relatedStateVariable>ExternalIPAddress</relatedStateVariable> 
  </argument>
  </argumentList>
  </action>
*/
#define GET_EXTERNALIP_PARAMS	"<NewExternalIPAddress></NewExternalIPAddress>\r\n"


#define wanIP  "WANIPConnection:1"
#define wanPPP  "WANPPPConnection:1"

extern void syslog(std::string info);

CUPnpNatController::CUPnpNatController()
{
	_state=CS_INIT;
    _action = AC_ADDPORT;
    m_pParent = NULL;
}

CUPnpNatController::~CUPnpNatController()
{

    Close();
	
}

void CUPnpNatController::setControlUrl( const char*controlUrl, const char* service )
{
    _controlUrl = controlUrl;
    _service = service;
}

//void CUPnpNatController::setAction(const char* action, unsigned short port, const char* protocol)
void CUPnpNatController::setAction( TUPnpAction act, unsigned short port, std::string protocol )
{

    if ( act == AC_ADDPORT )
        _actionName = "AddPortMapping";
    else if ( act == AC_DELPORT )
        _actionName = "DeletePortMapping";
    else //AC_GETIP
        _actionName = "GetExternalIPAddress";

    _action = act;

    _port = port;

    _protocol = protocol;
}

void CUPnpNatController::start()
{
    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _controlUrl.c_str(), ip, port, path );

    _state = CS_WORKING;

    CreateSock();

	assert(m_pDealer!=NULL);

	SockLib::TInetAddr4 dest;
	dest.iip=inet_addr(ip.c_str());
	dest.iport=htons(port);

    Connect( dest ,6000);

}

void CUPnpNatController::stop()
{

    Close();

}

TControllerState CUPnpNatController::getState()
{
    return _state;
}

void CUPnpNatController::OnRead()
{

	SockLib::CTCPClientSock::OnRead();
	if(!CanRead()) return;

    char buffer[ 4096 ];

    while ( 1 )
    {
        int ret = recv( m_hSocket, buffer, 4096, 0 );

        if ( ret == 0 )
        {
            OnClose();
            return ;
        }

        else if ( ret < 0 )
        {
			break;
        }

        else //if ( ret > 0 )
        {
            _recvBuffer.append( buffer, ret );
        }
    }

    return ;
}

void CUPnpNatController::OnWrite()
{
	SockLib::CTCPClientSock::OnWrite();
}

void CUPnpNatController::OnConnectOk()
{

    struct sockaddr_in addr;

#ifdef WIN32

    int len = sizeof( addr );
#else

    socklen_t len = sizeof( addr );
#endif

    maskWrite( false );

    getsockname( m_hSocket, ( struct sockaddr* ) & addr, &len );
    _localIP = inet_ntoa( addr.sin_addr );

    if ( _action == AC_ADDPORT )
        sendAddRequest();
    else if ( _action == AC_DELPORT )
        sendDelRequest();
    else
        sendGetipRequest();

    return;
}

void CUPnpNatController::OnConnectFail()
{

	_state = CS_ERROR;
	
	if ( _action == AC_ADDPORT )
	{
		assert(m_pParent!=NULL);
		m_pParent->NoticePortMapping( false, true, _port, _protocol,500 );
	}
	else if ( _action == AC_DELPORT )
	{
		assert(m_pParent!=NULL);
		m_pParent->NoticePortMapping( false, false, _port, _protocol,500 );
	}
	else
	{
		
		assert(m_pParent!=NULL);
		m_pParent->NoticeExternIP( false, "", 500 );
	}
}

void CUPnpNatController::OnClose()
{


	SockLib::CTCPClientSock::OnClose();

	size_t pos_1,pos_2;
	pos_1=pos_2=0;

	int ncode=500;

	while((pos_2=_recvBuffer.find('\n',pos_1))!=std::string::npos)
	{
		//found the pos2
		std::string line=_recvBuffer.substr(pos_1,pos_2-pos_1);
		pos_1=pos_2+1; //reset the pos_1
		Tools::TrimStringRight(line,"\r");
#if defined(WINCE)||defined(WIN32)
		if( line.length() >= 12 && _stricmp(line.substr(0,7).c_str(), "HTTP/1.") ==0)	
#else
		if( line.length() >= 12 && strcasecmp(line.substr(0,7).c_str(), "HTTP/1.") ==0)	
#endif
		{
			std::string code=line.substr(9,3);

			ncode=atoi(code.c_str());
			break;
		}
	}



	if(ncode/100!=2)
	{

        _state = CS_ERROR;

        if ( _action == AC_ADDPORT )
        {
           //assert(m_pParent!=NULL);
                m_pParent->NoticePortMapping( false, true, _port, _protocol,ncode );
        }
        else if ( _action == AC_DELPORT )
        {
            //assert(m_pParent!=NULL);
                m_pParent->NoticePortMapping( false, false, _port, _protocol,ncode );
        }
        else
        {

			//assert(m_pParent!=NULL);
				m_pParent->NoticeExternIP( false, "", ncode );
        }

	}
	else // ncode==2xx
    {

        _state = CS_OK;

        if ( _action == AC_ADDPORT )
        {
            m_pParent->NoticePortMapping( true, true, _port, _protocol,ncode );
        }
        else if ( _action == AC_DELPORT )
        {
            m_pParent->NoticePortMapping( true, false, _port, _protocol ,ncode);
        }
        else
        {

            //query the extern ip, should parse it
            /*
            HTTP/1.1 200 OK
            CONTENT-LENGTH:369
            CONTENT-TYPE:text/xml; charset="utf-8"
            EXT:
            SERVER:Ambit OS/1.0 UPnP/1.0 AMBIT-UPNP/1.0

            <?xml version="1.0"?>
            <s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle=
            "http://schemas.xmlsoap.org/soap/encoding/">
            <s:Body>
            <u:GetExternalIPAddressResponse xmlns:u="urn:schemas-upnp-org:service:WA
            NIPConnection:1">
            <NewExternalIPAddress>220.175.229.150</NewExternalIPAddress>
            </u:GetExternalIPAddressResponse>
            </s:Body>
            </s:Envelope>
            */

			int length=_recvBuffer.size();

            size_t pos1 = _recvBuffer.find( "<NewExternalIPAddress>" );
            size_t pos2 = _recvBuffer.find( "</NewExternalIPAddress>" );

            if ( pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
            {


                std::string ip = _recvBuffer.substr( strlen( "<NewExternalIPAddress>" ) + pos1, pos2 - pos1 - strlen( "<NewExternalIPAddress>" ) );
                m_pParent->NoticeExternIP( true, ip, ncode );

            }
			else
			{
				m_pParent->NoticeExternIP( false, "", ncode );
			}

        }
    }


}

void CUPnpNatController::OnTimer( unsigned int id )
{
	SockLib::CTCPClientSock::OnTimer(id);

}

void CUPnpNatController::sendAddRequest()
{

    char actbuf[ 1024 * 2 ];

//Õâžö×¢ÊÍ»á±»Ä³Ð©Â·ÓÉÆ÷×ö¹ØŒüÖµ£¬ÏàÍ¬µÄ×¢ÊÍ²»ÄÜÓÃÓÚœšÁ¢¶àžöÓ°Éä£¬·ñÔòÖ»ÄÜ³É¹ŠÒ»žö
//ÕâÑùÃèÊö×ÖŽ®ŸÍ±ØÐëÃ¿žö¶Œ²»Í¬£¬ÎªÁËŽïµœÄ¿µÄ£¬ÒÔ¶Ë¿ÚÓ³ÉäµÄ²ÎÊýÎª²ÎÊýœšÁ¢ÕâžöÃèÊöŽ®
//²Î¿Œ£º
//#define PORT_MAPPING_DESCRIPTION "Benliud [%d/%s]"


    sprintf( actbuf,
			 ADD_PORT_MAPPING_PARAMS,
             _port,
             _protocol.c_str(),
             _port,
             _localIP.c_str(),
			 _port,		//Õâžö²ÎÊýÓÃÓÚ×¢ÊÍÖÐ
			 _protocol.c_str() //Õâžö²ÎÊýÓÃÓÚ×¢ÊÍÖÐ
			 );


    char soapbuf[ 1024 * 4 ];


    sprintf( soapbuf,
			SOAP_ACTION,
             _actionName.c_str(),
             _service.c_str(),
             actbuf,
             _actionName.c_str() );



    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _controlUrl.c_str(), ip, port, path );

    char pocbuf[ 256 ];


    sprintf( pocbuf,  "%s:%u", ip.c_str(), port );


    char headbuf[ 1024 ];

    sprintf( headbuf,
			HTTP_HEADER_ACTION,
             path.c_str(),
             pocbuf,
             _service.c_str(),
             _actionName.c_str(),
             strlen( soapbuf ) );


    char totalbuf[ 1024 * 8 ];
    sprintf( totalbuf,  "%s%s", headbuf, soapbuf );

    int ret = send( m_hSocket, totalbuf, strlen( totalbuf ), 0 );

    maskRead( true );
}

void CUPnpNatController::sendDelRequest()
{

    char actbuf[ 1024 * 2 ];

    sprintf( actbuf,  DELETE_PORT_MAPPING_PARAMS,
             _port,
             _protocol.c_str() );


    char soapbuf[ 1024 * 4 ];


    sprintf( soapbuf, SOAP_ACTION,
             _actionName.c_str(),
             _service.c_str(),
             actbuf,
             _actionName.c_str() );


    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _controlUrl.c_str(), ip, port, path );

    char pocbuf[ 256 ];

    sprintf( pocbuf, "%s:%u", ip.c_str(), port );


    char headbuf[ 1024 ];


    sprintf( headbuf, HTTP_HEADER_ACTION,
             path.c_str(),
             pocbuf,
             _service.c_str(),
             _actionName.c_str(),
             strlen( soapbuf ) );


    char totalbuf[ 1024 * 8 ];

    sprintf( totalbuf, "%s%s", headbuf, soapbuf );

    int ret = send( m_hSocket, totalbuf, strlen( totalbuf ), 0 );

    maskRead( true );
}

void CUPnpNatController::sendGetipRequest()
{


    char actbuf[ 1024 ];
    actbuf[ 0 ] = 0;


    strcpy( actbuf, GET_EXTERNALIP_PARAMS );


    char soapbuf[ 1024 * 4 ];

    sprintf( soapbuf,  SOAP_ACTION,
             _actionName.c_str(),
             _service.c_str(),
             actbuf,
             _actionName.c_str() );

    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _controlUrl.c_str(), ip, port, path );



    char pocbuf[ 256 ];

    sprintf( pocbuf,  "%s:%u", ip.c_str(), port );


    char headbuf[ 1024 ];

    sprintf( headbuf, HTTP_HEADER_ACTION,
             path.c_str(),
             pocbuf,
             _service.c_str(),
             _actionName.c_str(),
             strlen( soapbuf ) );


    char totalbuf[ 1024 * 8 ];

    sprintf( totalbuf,"%s%s", headbuf, soapbuf );

    int ret = Send( totalbuf, strlen( totalbuf ));

    maskRead( true );
}

void CUPnpNatController::SetParent( CUPnpNat *parent )
{
    m_pParent = parent;
}

bool CUPnpNatController::parseUrl( const char* url, std::string& host, unsigned short& port, std::string& path )
{
    std::string str_url = url;

    std::string::size_type pos = str_url.find( "://" );

    if ( pos == std::string::npos )
    {
        return false;
    }

    str_url.erase( 0, pos + 3 );

    pos = str_url.find( ":" );

    if ( pos == std::string::npos )
    {
        port = 80;
        pos = str_url.find( "/" );

        if ( pos == std::string::npos )
        {
            return false;
        }

        host = str_url.substr( 0, pos );

        str_url.erase( 0, pos );
    }
    else
    {
        host = str_url.substr( 0, pos );

        str_url.erase( 0, pos + 1 );

        pos = str_url.find( "/" );

        if ( pos == std::string::npos )
        {
            return false;
        }

        std::string str_port = str_url.substr( 0, pos );
        port = ( unsigned short ) atoi( str_port.c_str() );

        str_url.erase( 0, pos );
    }

    if ( str_url.length() == 0 )
    {
        path = "/";
    }
    else
    {
        path = str_url;
    }

    return true;
}

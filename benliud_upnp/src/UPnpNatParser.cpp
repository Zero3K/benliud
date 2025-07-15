
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "../include/UPnpNatParser.h"
#include "../../thirdparty/tinyxml/tinyxml.h"
#include "../include/UpnpNatParserHandler.h"

#include <Dealer.h>
#include <Tools.h>

#if  !defined(WIN32) && !defined(WINCE)
#include <sys/types.h>
#include <sys/socket.h>
#else
#endif

#if !defined(WINCE)
#include <errno.h>
#endif

extern UINT	gStateCode;	//ϵͳ״̬��

#define wanIP  "WANIPConnection:1"
#define wanPPP  "WANPPPConnection:1"
#define SEARCH_REQUEST_DEVICE "urn:schemas-upnp-org:service:%s"

CUPnpNatParser::CUPnpNatParser()
        : _eventHandler( NULL )
        , _state( DPS_INIT )
{}

CUPnpNatParser::~CUPnpNatParser()
{}

void CUPnpNatParser::setDescriptionUrl( std::string& url )
{
    _url = url;
}

void CUPnpNatParser::setEventHandler( CUPnpNatParserHandler* handler )
{
    _eventHandler = handler;
}

void CUPnpNatParser::Start()
{
    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _url.c_str(), ip, port, path );

	
	gStateCode=40; //connecting description url
    if(!CreateSock())
	{
		OutputDebugString(L"fail to create socket");
		return ;
	}

	SockLib::TInetAddr4 dest(inet_addr(ip.c_str()),htons(port));
	
    SockLib::CTCPClientSock::Connect( dest, 10*1000 );
 
    _state = DPS_WORKING;
}

void CUPnpNatParser::Stop()
{

    SockLib::CTCPClientSock::Close();

}

void CUPnpNatParser::OnRead()
{


	SockLib::CTCPClientSock::OnRead();
	if(!CanRead()) {
		return;
	}

    char buffer[ 4096 ];

    while ( 1 )
    {
        int ret = recv( m_hSocket, buffer, 4096, 0 );

        if ( ret == 0 )
        {
			OnClose(); //parse it
            return ;
        }
        else if ( ret < 0 )
        {
            break;  //wait for next read
        }
        else// ( ret > 0 )
        {
            _recvBuffer.append( buffer, ret );
        }
    }

    return ;
}

void CUPnpNatParser::OnWrite()
{

	CTCPClientSock::OnWrite();
}

void CUPnpNatParser::OnConnectOk()
{

    sendRequest();
}

void CUPnpNatParser::OnConnectFail()
{

	_state = DPS_ERROR;
	Close();
}

void CUPnpNatParser::OnClose()
{


    bool parseResult = false;

    if ( _recvBuffer.size() > 0 )
    {
        char str[ 1024 ];
        std::string ip;
        std::string path;
        unsigned short port;
        parseUrl( _url.c_str(), ip, port, path );
#ifdef WINCE
		sprintf( str, "http://%s:%d", ip.c_str(), port );
#elif defined( WIN32)
        sprintf_s( str, 1024, "http://%s:%d", ip.c_str(), port );
#else
        sprintf( str, "http://%s:%d", ip.c_str(), port );
#endif
        parseResult = ParseResponse( _recvBuffer, str, wanIP );

        if ( !parseResult )
        {
            parseResult = ParseResponse( _recvBuffer, str, wanPPP );
        }
    }

    if ( !parseResult )
    {
        _state = DPS_ERROR;
    }
    else
    {
        _state = DPS_OK;
    }

    Close();

}

void CUPnpNatParser::OnTimer( unsigned int id )
{
	
	CTCPClientSock::OnTimer(id);
}

void CUPnpNatParser::sendRequest()
{
    std::string ip;
    std::string path;
    unsigned short port;
    parseUrl( _url.c_str(), ip, port, path );

    char httpRequest[ 1024 ];

#ifdef WINCE
    sprintf( httpRequest, 
             "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n",
             path.c_str(), ip.c_str(), port );
#elif defined( WIN32)
    sprintf_s( httpRequest, 1024,
             "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n",
             path.c_str(), ip.c_str(), port );
#else //wince and linux
    sprintf( httpRequest, 
             "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n",
             path.c_str(), ip.c_str(), port );
#endif

    Send( httpRequest, strlen( httpRequest ));

    maskRead( true );

}

bool CUPnpNatParser::ParseResponse( std::string&  response, const char* httpUrl, const char* constServiceType )
{

    //the response is with header, find the header pos first;
	//split the header and body

	std::string header, content;

	size_t headpos=response.find("\r\n\r\n");
	if(headpos==std::string::npos) {
		gStateCode=41;
		return false;
	}

	header=response.substr(0,headpos+2); // header will end with "\r\n";
	content=response.substr(headpos+4);

	size_t fl=header.find("\r\n");
	if(fl==std::string::npos) {
		gStateCode=42;
		return false;
	}

	std::string fline=header.substr(0,fl);

	int ncode=atoi(fline.substr(9,3).c_str());
    //check the header for "HTTP/1.1 200 OK"
	if(ncode/100!=2) {
		gStateCode=43;
		return false;
	}

    TiXmlDocument document;

    document.Parse( content.c_str() ); //xml buffer NULL ended

    if ( document.Error() )
    {
		gStateCode=44;
        return false;
    }


	TiXmlHandle rootHandle( document.RootElement() );

    std::string baseURL;

    TiXmlElement * urlbase = rootHandle.FirstChildElement( "URLBase" ).ToElement();

    if ( urlbase && urlbase->GetText()) //the gettext() will return NULL if don't have content
    {
        baseURL = urlbase->GetText();

		if(baseURL[baseURL.length()-1]!='/') 
		{
			baseURL+="/";
		}
    }
    else
    {
        baseURL = httpUrl;

		if(baseURL[baseURL.length()-1]!='/') 
		{
			baseURL+="/";
		}
    }

    //enum all device to find out we want
    int deviceCount = 0;

    bool found = false;

    TiXmlElement *devEle;

    do
    {

        TiXmlHandle deviceHandle = rootHandle.Child( "device", deviceCount );
        devEle = deviceHandle.ToElement();

        if ( devEle )
        {
            TiXmlHandle devType( devEle );
            TiXmlElement *typeEle = devType.FirstChildElement( "deviceType" ).ToElement();

            if ( typeEle==NULL || (typeEle!=NULL && typeEle->GetText()==NULL))
            {
                continue;
            }
            else if ( strcmp( typeEle->GetText(), "urn:schemas-upnp-org:device:InternetGatewayDevice:1" ) == 0 )
            { //we found intreasting device type
                found = true;
                break;
            };

            deviceCount++;
        }
    }
    while ( devEle );

    if ( !found )
	{
		gStateCode=45;
		return false; //no device
	}

    TiXmlHandle deviceHandle = rootHandle.Child( "device", deviceCount ); //this is router device

    //find for device list
    TiXmlHandle deviceListHandle = deviceHandle.FirstChild( "deviceList" );

    //in a router devicelist maybe many device, so just find what we need.
    deviceCount = 0;

    found = false;

    do
    {

        TiXmlHandle deviceHandle = deviceListHandle.Child( "device", deviceCount );
        devEle = deviceHandle.ToElement();

        if ( devEle )
        {
            TiXmlHandle devType( devEle );
            TiXmlElement *typeEle = devType.FirstChildElement( "deviceType" ).ToElement();

            if ( typeEle==NULL || (typeEle!=NULL && typeEle->GetText()==NULL))
            {
                continue;
            }
            else if ( strcmp( typeEle->GetText(), "urn:schemas-upnp-org:device:WANDevice:1" ) == 0 )
            { //we found intreasting device type

                found = true;
                break;
            };

            deviceCount++;
        }
    }
    while ( devEle );

    if ( !found )
	{
		gStateCode=46;
		return false;
	}

    deviceHandle = deviceListHandle.Child( "device", deviceCount ); //this is deviceList's device, it have servicelist node and devicelist node

    //again find devicelist.
    deviceListHandle = deviceHandle.FirstChild( "deviceList" );

    deviceCount = 0;

    found = false;

    do
    {

        TiXmlHandle deviceHandle = deviceListHandle.Child( "device", deviceCount );
        devEle = deviceHandle.ToElement();

        if ( devEle )
        {
            TiXmlHandle devType( devEle );
            TiXmlElement *typeEle = devType.FirstChildElement( "deviceType" ).ToElement();

            if ( typeEle==NULL || (typeEle!=NULL && typeEle->GetText()==NULL ))
            {
                continue;
            }
            else if ( strcmp( typeEle->GetText(), "urn:schemas-upnp-org:device:WANConnectionDevice:1" ) == 0 )
            { //we found intreasting device type

                found = true;
                break;
            };

            deviceCount++;
        }
    }
    while ( devEle );

    if ( !found )
	{
		gStateCode=47;
		return false;
	}

    deviceHandle = deviceListHandle.Child( "device", deviceCount );

    TiXmlHandle serviceListHandle = deviceHandle.FirstChild( "serviceList" );

    //search in service list

    int serviceCount = 0;

    found = false;

    //const char* constserviceType="WANIPConnection:1";
    char compareType[ 256 ];
#ifdef WINCE
	sprintf( compareType,  SEARCH_REQUEST_DEVICE, constServiceType );
#elif defined( WIN32)
    sprintf_s( compareType, 256, SEARCH_REQUEST_DEVICE, constServiceType );
#else
    sprintf( compareType,  SEARCH_REQUEST_DEVICE, constServiceType );
#endif
    TiXmlElement *serEle;

    do
    {

        TiXmlHandle serviceHandle = serviceListHandle.Child( "service", serviceCount );
        serEle = serviceHandle.ToElement();

        if ( serEle )
        {
            TiXmlHandle devType( serEle );
            TiXmlElement *sEle = devType.FirstChildElement( "serviceType" ).ToElement();

            if ( sEle==NULL || ( sEle!=NULL && sEle->GetText()==NULL ))
            {
                continue;
            }
            else if ( strcmp( sEle->GetText(), compareType ) == 0 )
            { //we found intreasting device type

                found = true;
                break;
            };

            serviceCount++;
        }
    }
    while ( serEle );

    if ( !found )
	{
		gStateCode=48;
		return false;
	}

    //and at last ,get the service's controlURL
    TiXmlHandle serviceHandle = serviceListHandle.Child( "service", serviceCount );

    TiXmlElement* ctrlurl = serviceHandle.FirstChildElement( "controlURL" ).ToElement();

    if ( ctrlurl==NULL || (ctrlurl!=NULL && ctrlurl->GetText()==NULL))
    {
		gStateCode=49;
		return false;
	}

    std::string ControlUrl = ctrlurl->GetText();


	Tools::TrimString(ControlUrl);

#if defined( WIN32)||defined(WINCE)
    if ( _strnicmp( ControlUrl.c_str(), "HTTP://", 7 ) != 0 )
#else
	if ( strncasecmp( ControlUrl.c_str(), "HTTP://", 7 ) != 0 )
#endif
    {
		//if baseUrl ended with '\' and ControlUrl begin with '\'
		//then we need to remove a '\' or we will get a url like http://192.168.1.1:80//upnp/service/WANPPPConnection
		//then when we paser this url may got the request address = //upnp/service/WANPPPConnection
		//on some router it will ok, but some router it will fail.

		//baseUrl alwayse ended with '\' so just check if ContronUrl begin with '\'

		if(ControlUrl[0]=='/')
		{
			ControlUrl.erase(0,1);
			ControlUrl=baseURL + ControlUrl;
		}
		else
		{

			ControlUrl = baseURL + ControlUrl;

		}

    }

    _eventHandler->OnGetControlUrl( ControlUrl.c_str(), constServiceType );

    return true;
}

TDescriptionParserState CUPnpNatParser::getState()
{
    return _state;
}

bool CUPnpNatParser::parseUrl( const char* url, std::string& host, unsigned short& port, std::string& path )
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

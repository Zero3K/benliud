/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _UPNPNATPARSERHANDLER_H
#define _UPNPNATPARSERHANDLER_H

#include <string>

class CUPnpNatParserHandler
{

public:
    virtual ~CUPnpNatParserHandler()
    {}

    ;
    virtual void OnGetControlUrl( const char* controlUrl, const char* service ) = 0;
};

#endif



/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _UPNPNATFINDERHANDLER_H
#define _UPNPNATFINDERHANDLER_H

#include <string>
class CUPnpNatFinderHandler
{

public:
    virtual ~CUPnpNatFinderHandler()
    {}

    ;

    virtual void OnGetDescriptionUrl( std::string& url ) = 0;
};

#endif


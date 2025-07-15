/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


#ifndef _TPIECECACHE_H
#define _TPIECECACHE_H

#include <string>
#include <map>

struct TPieceCache
{
    std::string data;
    unsigned int lastAccessTick;
};

typedef std::map<unsigned int, TPieceCache> TReadPieceMap;

#endif


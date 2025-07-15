/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

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


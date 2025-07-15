/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
// BTPieceSum.h: interface for the CBTPieceSum class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BTPIECESUM_H
#define _BTPIECESUM_H

#include "BTPiece.h"

#include <vector>

class CBTPieceSum
{

public:
    unsigned int GetSize();
    void SetValue( int index, int value );
    unsigned int GetSetCount();
    unsigned int GetUnsetCount();
    int GetValue( int index );
    void NewPiece( int index );
    void Init( unsigned int size, int value = 0 );
    CBTPieceSum();
    virtual ~CBTPieceSum();
    void operator+=( CBTPiece& peer );
    void operator-=( CBTPiece& peer );
    void operator= ( CBTPieceSum& peer );

protected:
    std::vector<int> m_data;
    unsigned int m_size;
};

#endif

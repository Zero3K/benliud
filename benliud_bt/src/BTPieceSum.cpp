/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

// BTPieceSum.cpp: implementation of the CBTPieceSum class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/BTPieceSum.h"
#include <assert.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBTPieceSum::CBTPieceSum()
{
}

CBTPieceSum::~CBTPieceSum()
{
}

void CBTPieceSum::Init( unsigned int size, int value )
{
    m_size = size;
    m_data.resize( size );

    for ( unsigned int i = 0;i < size;i++ )
    {
        m_data[ i ] = value;
    }
}

void CBTPieceSum::operator +=( CBTPiece& peer )
{
    for ( unsigned int i = 0;i < m_size;i++ )
    {
        if ( peer.IsSet( i ) )
        {
            m_data[ i ] ++;
        }
    }
}

void CBTPieceSum::operator -=( CBTPiece& peer )
{
    for ( unsigned int i = 0;i < m_size;i++ )
    {
        if ( peer.IsSet( i ) )
        {
            if ( m_data[ i ] > 0 )
                m_data[ i ] --;
        }
    }
}

void CBTPieceSum::operator =( CBTPieceSum& piece )
{
    m_size = piece.GetSize();
    m_data.resize( m_size );

    for ( unsigned int i = 0;i < m_size;i++ )
    {
        m_data[ i ] = piece.GetValue( i );
    }
}

void CBTPieceSum::NewPiece( int index )
{
	assert(index>=0 && index<m_size);

	if(index<0||index >=m_size) return;

    m_data[ (unsigned int)index ] ++;
}

int CBTPieceSum::GetValue( int index )
{
	if(index<0||index >= m_size) return 0;
    return m_data[ (unsigned int)(index) ];
}

//get the sum of 0
unsigned int CBTPieceSum::GetUnsetCount()
{
    unsigned int count = 0;

    for ( unsigned int i = 0;i < m_size;i++ )
    {
        if ( !m_data[ i ] )
        {
            count++;
        }
    }

    return count;
}

unsigned int CBTPieceSum::GetSetCount()
{
    unsigned int count = 0;

    for ( unsigned int i = 0;i < m_size;i++ )
    {
        if ( m_data[ i ] )
        {
            count++;
        }
    }

    return count;
}

void CBTPieceSum::SetValue( int index, int value )
{
	if(index<0||index >= m_size) return; 
    m_data[ (unsigned int)index ] = value;
}

unsigned int CBTPieceSum::GetSize()
{
    return m_size;
}

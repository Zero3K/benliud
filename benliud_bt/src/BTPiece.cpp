/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

#include "../include/BTPiece.h"
#include <assert.h>

static unsigned int bittable[256]=
{
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,
2,3,3,4,2,3,3,4,3,4,4,5,1,2,2,3,2,3,3,4,
2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,
4,5,5,6,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,2,3,3,4,
3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,
4,5,5,6,5,6,6,7,1,2,2,3,2,3,3,4,2,3,3,4,
3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,
4,5,5,6,4,5,5,6,5,6,6,7,2,3,3,4,3,4,4,5,
3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,
5,6,6,7,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};

CBTPiece::CBTPiece()
{
    m_size = 0;
}

void CBTPiece::operator=( CBTPiece& bitset )
{
    m_size = bitset.GetSize();
    m_data = bitset.GetStream();
}

bool CBTPiece::operator>=( CBTPiece& other)
{
	/*
	//otherõλǶ
	if(other.GetSize()!=m_size) return false;

	for(unsigned int i=0;i< m_size;i++)
	{
		if(other.IsSet(i) && !IsSet(i))
		{
			return false;
		}
	}

	return true;
	*/

	//ȽϿܷܺCPUЩbitset5000
	//ýƼʡCPU, ÿbyteеλ>=other>=other
	if(other.GetSize()!=m_size) return false;

	unsigned int bytes=m_data.size();

	for(unsigned int i=0;i<bytes;i++)
	{
		if( bittable[(unsigned char)(m_data[i])] < bittable[(unsigned char)(other.m_data[i])])
		{
			return false;
		}
	}

	return true; //۲CPU

}

CBTPiece::CBTPiece( CBTPiece& bitset )
{
    m_size = bitset.GetSize();
    m_data = bitset.GetStream();
}

CBTPiece::~CBTPiece()
{}

void CBTPiece::Init( unsigned int size )
{
    m_size = size;

    if ( size % 8 == 0 )
    {
        m_data.resize( size / 8 );
    }
    else
    {
        m_data.resize( size / 8 + 1 );
    }

    for ( unsigned int i = 0; i < m_data.size(); i++ )
    {
        m_data[ i ] = 0;
    }
}

void CBTPiece::Init( std::string& stream, unsigned int size )
{
    m_size = size;

	unsigned int len;

	if(m_size % 8 ==0)
	{
		len=size/8;
	}
	else
	{
		len=size/8+1;
	}

	assert(stream.size()==len);

	if(stream.size()!=len) Init(size);
	else   m_data = stream;
}

bool CBTPiece::IsSet( unsigned int index ) const
{

	if(index >= m_size) return true; 
		
	assert(index/8 < m_data.size());

	unsigned char  bit = index % 8;
	unsigned char  byte = m_data[ index / 8 ];
    unsigned char  mask = ( 0x80 >> bit );
    
	return (byte & mask) !=0;

}

void CBTPiece::Set( unsigned int index, bool set )
{

	if(index >= m_size) return;

    unsigned char b = m_data[ index / 8 ];

    unsigned int bit = index % 8;

    unsigned char mask = 128;
    mask = mask >> bit;

    if ( set )
    {
        b = b | mask;
    }
    else
    {
        mask = ~mask;
        b = b & mask;
    }

    m_data[ index / 8 ] = b;
}

std::string& CBTPiece::GetStream()
{
    return m_data;
}

bool CBTPiece::IsAllSet() 
{
    for ( size_t i = 0; i < m_data.size() - 1; ++i )
    {
        if ( ( unsigned char ) ( m_data[ i ] ) != 0xFF )
        {
            return false;
        }
    }

    unsigned char b = ( unsigned char ) m_data[ m_data.size() - 1 ];

    for ( unsigned int j = 0; j < m_size - ( m_data.size() - 1 ) * 8; ++j )
    {
        unsigned char mask = 128;
        mask = mask >> j;

        if ( ( b & mask ) == 0 )
        {
            return false;
        }
    }

    return true;
}

bool CBTPiece::IsEmpty()
{
    for ( unsigned int i = 0; i < m_data.size(); i++ )
    {
        if ( m_data[ i ] != 0 )
        {
            return false;
        }
    }

    return true;
}

unsigned int CBTPiece::GetSize() const
{
    return m_size;
}

unsigned int CBTPiece::GetSetedCount()
{
	//

	unsigned int seted=0;
	for(unsigned int i=0;i<m_data.size();i++)
	{
		seted+= bittable[(unsigned char)(m_data[i])];
	}

	return seted;

}

void CBTPiece::SetAll()
{

    for ( unsigned int i = 0; i < m_size / 8; i++ )
    {
        m_data[ i ] = (unsigned char)( 0xFF );
    }

	if ( m_size % 8)
	{
        //left bit
        unsigned int begin = m_size - m_size % 8;

        for ( unsigned int j = begin ; j < m_size; j++ )
        {
            Set( j, true );
        }
	}

}

//return the number of piece we have but other don't have
int CBTPiece::operator-(const CBTPiece& other)
{
	unsigned int count=0;

	if(m_size!=other.GetSize()) return 0;
		
	for(unsigned int i=0;i<m_size;i++)
	{
		if(IsSet(i) && !other.IsSet(i)) count++;
	}
	
	return count;
}

float CBTPiece::GetPersent()
{
	if(m_size==0) return 0.0f;
	return float(GetSetedCount())/float(m_size);
}

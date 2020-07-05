/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#ifndef _BTPIECE_H
#define _BTPIECE_H

#include <string>

class CBTPiece
{

public:
    CBTPiece();
    CBTPiece( CBTPiece& bitset );
    virtual ~CBTPiece();
	void operator=(CBTPiece& bitset);
	int operator-(const CBTPiece& other);
	bool operator>=( CBTPiece& bitset);

protected:
    std::string m_data;
    unsigned int m_size;

public:
	

    void SetAll();
    void Init( unsigned int size );
    void Init( std::string& stream, unsigned int size );
    bool IsSet( unsigned int index ) const;

    void Set( unsigned int index, bool set  );
	float GetPersent();
    bool IsAllSet();

    bool IsEmpty();

    unsigned int GetSetedCount();

    std::string& GetStream();

    unsigned int GetSize() const;
};

#endif

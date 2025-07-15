/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#include <time.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include "../include/MSE_BTDHTKey.h"
#include <Tools.h>


namespace MSE
{
BTDHTKey::BTDHTKey()
{
	for (int i = 0; i < 20 ;i++)
		hash[i] = 0;
}

BTDHTKey::BTDHTKey(const BTDHTKey& other)
{
	for (int i = 0; i < 20 ;i++)
		hash[i] = other[i];
}

BTDHTKey::BTDHTKey(const char* ba)
{
	for (int i = 0; i < 20 ;i++)
		hash[i] = ba[i];
}

BTDHTKey::~BTDHTKey()
{}

bool BTDHTKey::operator == (const BTDHTKey & other) const
{
	for (int i=0;i<20;i++)
	{
		if(hash[i]!=other.hash[i]) return false;
	}		
	return true;
}

bool BTDHTKey::operator != (const BTDHTKey & other) const
{
	for (int i=0;i<20;i++)
	{
		if(hash[i]!=other.hash[i]) return true;
	}		
	return false;
}

bool BTDHTKey::operator < (const BTDHTKey & other) const
{
	for (int i = 0;i < 20;i++)
	{
		if (hash[i] < other.hash[i])
			return true;
		else if (hash[i] > other.hash[i])
			return false;
	}
	return false;
}

bool BTDHTKey::operator <= (const BTDHTKey & other) const
{
	return operator < (other) || operator == (other);
}

bool BTDHTKey::operator > (const BTDHTKey & other) const
{
	for (int i = 0;i < 20;i++)
	{
		if (hash[i] < other.hash[i])
			return false;
		else if (hash[i] > other.hash[i])
			return true;
	}
	return false;
}

bool BTDHTKey::operator >= (const BTDHTKey & other) const
{
	return operator > (other) || operator == (other);
}


BTDHTKey BTDHTKey::operator - (const BTDHTKey& other)
{
	BTDHTKey k;
	for (int i = 0;i < 20;i++)
	{
		k.hash[i] = hash[i] ^ other.hash[i];
	}
	return k;
}


void BTDHTKey::operator=(const BTDHTKey& other)
{
	for (int i = 0;i < 20;i++)
	{
		hash[i] = other[i];
	}	
}

void BTDHTKey::Random()
{
	srand(GetTickCount());

	for (int i = 0;i < 20;i++)
	{
		hash[i] = (unsigned char)(rand() % 0xFF);
	}

}


unsigned char* BTDHTKey::GetData()
{
	return hash;
}

};

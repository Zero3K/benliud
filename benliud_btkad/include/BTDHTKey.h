/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _BTDHTKEY_H
#define _BTDHTKEY_H
#ifdef WIN32
#pragma warning(disable: 4786)
#endif
#include <Tools.h>
class BTDHTKey 
{
public:
	unsigned char* GetData();

	BTDHTKey();
	
	BTDHTKey(const BTDHTKey& other);

	BTDHTKey(const char* pbuf);
	
	virtual ~BTDHTKey();
	
	void Random();
	
	void operator=(const BTDHTKey& other);

	bool operator == (const BTDHTKey & other) const;
	
	bool operator != (const BTDHTKey & other) const;
	
	bool operator < (const BTDHTKey & other) const;

	bool operator <= (const BTDHTKey & other) const;
	
	bool operator > (const BTDHTKey & other) const;
	
	bool operator >= (const BTDHTKey & other) const;

	BTDHTKey operator-(const BTDHTKey& other);
	
	unsigned char operator [] (const unsigned int idx) const 
	{
		return idx < 20 ? hash[idx] : 0;
	}
protected:
	unsigned char hash[20];
};


#endif

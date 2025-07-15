
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
#ifndef _MSE_BTDHTKEY_H
#define _MSE_BTDHTKEY_H

namespace MSE
{
class BTDHTKey 
{
public:
	unsigned char* GetData();

	BTDHTKey();
	
	BTDHTKey(const BTDHTKey& other);

	BTDHTKey(const char* ba);
	
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

};

#endif

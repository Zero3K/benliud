/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#ifndef _MSE_BIGINT_H
#define _MSE_BIGINT_H


#include <string>
#include <stdio.h>
#include "../../thirdparty/bigint/bigint.h"

namespace MSE
{

class BigInt
{
public:
	void Printf(int base=10);
	BigInt();

	BigInt(unsigned int num_bits );
	

	BigInt(const std::string & value,int base=0);
	

	BigInt(const BigInt & bi);
	virtual ~BigInt();
	

	BigInt & operator = (const BigInt & bi);
	
	static BigInt powerMod(const BigInt & x,const BigInt & e,const BigInt & d);
	
	/// Make a random BigInt
	static BigInt random();
	
	/// Export the bigint ot a buffer
	unsigned int toBuffer(unsigned char* buf,unsigned int max_size) const;
	
	/// Make a BigInt out of a buffer
	static BigInt fromBuffer(const unsigned char* buf,unsigned int size);
	
	static unsigned char ToBinaryChar(char h,char l);
	static unsigned char ToBinaryChar(char h);

private:
	BigInteger val;
};

};//namespace

#endif

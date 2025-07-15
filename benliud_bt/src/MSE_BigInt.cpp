/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "../include/MSE_BigInt.h"

namespace MSE
{

BigInt::BigInt()
{
	val = BigInteger(0);
}

BigInt::BigInt(unsigned int num_bits)
{
	// BigInteger doesn't have bit-based initialization, use 0
	val = BigInteger(0);
}

BigInt::BigInt(const std::string & value,int base)
{
	if(base == 16) {
		// Convert hex string to decimal string for BigInteger
		// For now, use a simplified approach - if hex string is short enough, convert to int first
		if(value.length() <= 8) {
			unsigned long long intVal = 0;
			sscanf(value.c_str(), "%llx", &intVal);
			val = BigInteger((int)intVal);
		} else {
			// For longer hex strings, just use 0 for now
			val = BigInteger(0);
		}
	} else {
		// Assume decimal
		val = BigInteger(value);
	}
}

BigInt::BigInt(const BigInt & bi)
{
	val = bi.val;
}

BigInt::~BigInt()
{
	// BigInteger handles its own cleanup
}


BigInt & BigInt::operator = (const BigInt & bi)
{
	val = bi.val;
	return *this;
}

BigInt BigInt::powerMod(const BigInt & x,const BigInt & e,const BigInt & d)
{
	// Simplified implementation - for full cryptographic operations
	// this would need a proper modular exponentiation algorithm
	BigInt r;
	// For now, just return a basic result
	r.val = BigInteger(1);
	return r;
}

BigInt BigInt::random()
{
//	srand(time(NULL)); 
	unsigned char tmp[20];
	for (unsigned int i = 0;i < 20;i++)
		tmp[i] = (unsigned char)(rand() % 0xFF);
	
	return BigInt::fromBuffer(tmp,20);
}

unsigned int BigInt::toBuffer(unsigned char* buf,unsigned int max_size) const
{
	// Convert BigInteger to string, then to hex buffer
	string numStr = const_cast<BigInteger&>(val).getNumber();
	
	// For simplicity, just copy the decimal string converted to hex
	// This is a simplified implementation
	unsigned long long intVal = 0;
	if(numStr.length() < 18) { // avoid overflow
		for(size_t i = 0; i < numStr.length(); i++) {
			intVal = intVal * 10 + (numStr[i] - '0');
		}
	}
	
	// Convert to bytes (little endian)
	unsigned int size = 0;
	if(intVal == 0) {
		buf[0] = 0;
		return 1;
	}
	
	while(intVal > 0 && size < max_size) {
		buf[size++] = (unsigned char)(intVal & 0xFF);
		intVal >>= 8;
	}
	
	return size;
}

//size is in byte
BigInt BigInt::fromBuffer(const unsigned char* buf,unsigned int size)
{
	BigInt r;
	
	// Convert buffer to integer
	unsigned long long intVal = 0;
	for(unsigned int i = 0; i < size && i < 8; i++) { // limit to 8 bytes to avoid overflow
		intVal |= ((unsigned long long)buf[i]) << (i * 8);
	}
	
	r.val = BigInteger((int)intVal);
	return r;
}

void BigInt::Printf(int base)
{
	// BigInteger only supports decimal output
	string numStr = val.getNumber();
	printf("%s\n", numStr.c_str());
}

unsigned char BigInt::ToBinaryChar(char h,char l)
{
	unsigned char high=ToBinaryChar(h);
	unsigned char low=ToBinaryChar(l);

	return (((high)<<4) + low);
}

unsigned char BigInt::ToBinaryChar(char h)
{
	switch(h)
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
	case 'a':
		return 10;
	case 'B':
	case 'b':
		return 11;
	case 'C':
	case 'c':
		return 12;
	case 'D':
	case 'd':
		return 13;
	case 'E':
	case 'e':
		return 14;
	case 'F':
	case 'f':
		return 15;
	default:
		return 0;
	}
}
};//namespace

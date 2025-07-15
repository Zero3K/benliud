/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/

#include "stdafx.h"

#include "../include/MSE_Rc4encryptor.h"

//const unsigned int MAX_MSGLEN = 32*1024; //9 + 131072;

namespace MSE
{
	static void swap(unsigned char & a,unsigned char & b)
	{
		unsigned char tmp = a;
		a = b;
		b = tmp;
	}
	
//	static unsigned char rc4_enc_buffer[MAX_MSGLEN]; 
	
	RC4::RC4(const unsigned char* key,unsigned int size)
	{
		i=j=0;
		// initialize state
		for (unsigned int t = 0;t < 256;t++)
			s[t] = t;
		
		j = 0;
		for (unsigned int u=0;u < 256;u++)
		{
			j = (j + s[u] + key[u % size]) & 0xFF;
			swap(s[u],s[j]);
		}
		
		i = j = 0;
	}
	
	RC4::~RC4()
	{
	}
		
	void RC4::process(const unsigned char* in,unsigned char* out,unsigned int size)
	{
		for (unsigned int k = 0;k < size;k++)
		{
			out[k] = process(in[k]);
		}
	}
	
	unsigned char RC4::process(unsigned char b)
	{
		i = (i + 1) & 0xFF;
		j = (j + s[i]) & 0xFF;
		swap(s[i],s[j]);
		unsigned char tmp = s[ (s[i] + s[j]) & 0xFF];
		return tmp ^ b;
	}
	

	RC4Encryptor::RC4Encryptor( BTDHTKey & dk, BTDHTKey & ek) 
	: enc(ek.GetData(),20),dec(dk.GetData(),20)
	{
		unsigned char tmp[1024];
		enc.process(tmp,tmp,1024);
		dec.process(tmp,tmp,1024);
	}


	RC4Encryptor::~RC4Encryptor()
	{}


	void RC4Encryptor::decrypt(unsigned char* data,unsigned int len)
	{
		dec.process(data,data,len);
	}

	//encrypt to our place
	const unsigned char* RC4Encryptor::encrypt(const unsigned char* data,unsigned int len)
	{
		enc.process(data,rc4_enc_buffer,len);
		return rc4_enc_buffer;
	}
	
	//encrypt to original place
	void RC4Encryptor::encryptReplace(unsigned char* data,unsigned int len)
	{
		enc.process(data,data,len);
	}
	
}

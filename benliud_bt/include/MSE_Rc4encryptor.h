/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/
#ifndef _MSE_RC4ENCRYPTOR_H
#define _MSE_RC4ENCRYPTOR_H


#include "MSE_BTDHTKey.h"

namespace MSE
{

	class RC4
	{
		unsigned char i,j;
		unsigned char s[256];
	public:
		RC4(const unsigned char* key,unsigned int size);
		virtual ~RC4();
		
		void process(const unsigned char* in,unsigned char* out,unsigned int size);
		unsigned char process(unsigned char b);
	};


#define MAX_MSGLEN (32*1024)

	class RC4Encryptor
	{
		RC4 enc,dec;
		unsigned char rc4_enc_buffer[MAX_MSGLEN]; 

	public:

		RC4Encryptor( BTDHTKey & dkey, BTDHTKey & ekey);
		virtual ~RC4Encryptor();

		void decrypt(unsigned char* data,unsigned int len);

		const unsigned char* encrypt(const unsigned char* data,unsigned int len);

		void encryptReplace(unsigned char* data,unsigned int len);

		unsigned char encrypt(unsigned char b) {return enc.process(b);}
	};

}

#endif

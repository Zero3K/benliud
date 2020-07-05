/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

#include <time.h>
#include <stdlib.h>
#include "../include/MSE_BigInt.h"

namespace MSE
{

BigInt::BigInt()
{
	mpz_init(val);
}

BigInt::BigInt(unsigned int num_bits)
{
	mpz_init2(val,num_bits);
}

BigInt::BigInt(const std::string & value,int base)
{
	mpz_init_set_str(val,value.c_str(),base);
}

BigInt::BigInt(const BigInt & bi)
{
	mpz_init_set(val,bi.val);
}

BigInt::~BigInt()
{
	mpz_clear(val);
}


BigInt & BigInt::operator = (const BigInt & bi)
{
	mpz_clear(val);
	mpz_init_set(val,bi.val);
	return *this;
}

BigInt BigInt::powerMod(const BigInt & x,const BigInt & e,const BigInt & d)
{
	BigInt r;
	mpz_powm(r.val,x.val,e.val,d.val);
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
	static char sbuf[2048];
	mpz_get_str(sbuf, 16, val); //16 is base, get the number string

	//the string should ended with 0
	//and then convert the string to binary
	int bsize=strlen(sbuf)/2;

	if(strlen(sbuf)%2!=0) 
	{//单数前补零0FFF,这样才可以对齐字节
		buf[0]=ToBinaryChar('0',sbuf[0]);

		for(int i=0;i<bsize ;i++)
		{
			buf[i+1]=ToBinaryChar(sbuf[i*2+1],sbuf[i*2+2]);
		}	
		
		return (strlen(sbuf)+1) /2;
	}
	else
	{
		for(int i=0;i<bsize;i++)
		{
			buf[i]=ToBinaryChar(sbuf[i*2],sbuf[i*2+1]);
		}

		return bsize;
	}


//	mpz_export(buf,&foo,1,1,1,0,val);
	//convert the buf to string and use mpz_init_set_str
	//mpz_init_set_str(val,value.c_str(),base);

}

//size is in byte
BigInt BigInt::fromBuffer(const unsigned char* buf,unsigned int size)
{

	BigInt r;//(size*8);

//convert the buf to string and use mpz_init_set_str
	char *str=new char[size*2+2];
	memset(str,0,size*2+2);

	for(unsigned int i=0;i<size;i++)
	{
		sprintf(str+i*2,"%02X",buf[i]);
	}

	mpz_set_str(r.val,str,16);
	delete[] str;

//	mpz_import(r.val,size,1,1,1,0,buf);
	return r;

}

void BigInt::Printf(int base)
{
	static char buf[4096];
	mpz_get_str(buf, base, val); //10 is base
	printf("%s\n",buf);
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

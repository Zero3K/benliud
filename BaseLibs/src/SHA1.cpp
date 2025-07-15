/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/

//#include "StdAfx.h"
#include "../include/SHA1.h"


namespace HashLib
{
	CSHA1::CSHA1(void)
	{
	}

	CSHA1::CSHA1(const char* data, int len)
	{
		 SHA_CTX ctx;
		 SHA1_Init(&ctx);
		 SHA1_Update(&ctx, (const unsigned char*)data, len);
		 SHA1_Final(m_Hash, &ctx);
	}

	void CSHA1::Hash(const char* data, int len)
	{
		 SHA_CTX ctx;
		 SHA1_Init(&ctx);
		 SHA1_Update(&ctx, (const unsigned char*)data, len);
		 SHA1_Final(m_Hash, &ctx);
	}

	void CSHA1::Open()
	{
		SHA1_Init(&m_ctx);
	}
	void CSHA1::Close()
	{
		SHA1_Final(m_Hash, &m_ctx);
	}
	void CSHA1::Update(const char* data, int len)
	{
		SHA1_Update(&m_ctx, (const unsigned char*)data, len);
	}
}

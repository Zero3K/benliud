/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

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
		 SHA1_Update(&ctx, data, len);
		 SHA1_Final( m_Hash, &ctx);
	}

	void CSHA1::Hash(const char* data, int len)
	{
		 SHA_CTX ctx;
		 SHA1_Init(&ctx);
		 SHA1_Update(&ctx, data, len);
		 SHA1_Final( m_Hash, &ctx);
	}

	void CSHA1::Open()
	{
		SHA1_Init(&m_ctx);
	}
	void CSHA1::Close()
	{
		SHA1_Final( m_Hash, &m_ctx);
	}
	void CSHA1::Update(const char* data, int len)
	{
		SHA1_Update(&m_ctx, data, len);
	}
}

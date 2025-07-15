/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

//#include "StdAfx.h"
#include "../include/MD5.h"


namespace HashLib
{
	CMD5::CMD5(void)
	{
	}

	CMD5::CMD5(const char* data, int len)
	{
		MD5_CTX ctx;
		MD5_Init(&ctx);
		MD5_Update(&ctx, data, len);
		MD5_Final(m_Hash, &ctx);

	}

	void CMD5::Hash(const char* data, int len)
	{
		 MD5_CTX ctx;
		 MD5_Init(&ctx);
		 MD5_Update(&ctx, data, len);
		 MD5_Final( m_Hash, &ctx);
	}

	void CMD5::Open()
	{
		MD5_Init(&m_ctx);
	}

	void CMD5::Close()
	{
		MD5_Final( m_Hash, &m_ctx);
	}
	void CMD5::Update(const char* data, int len)
	{
		MD5_Update(&m_ctx, data, len);
	}
}

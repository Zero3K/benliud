/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
//#include "StdAfx.h"
#include "../include/RipeMd160.h"


namespace HashLib
{
	CRipeMd160::CRipeMd160(void)
	{
	}

	CRipeMd160::CRipeMd160(const char* data, int len)
	{
		 RIPEMD160_CTX ctx;
		 RIPEMD160_Init(&ctx);
		 RIPEMD160_Update(&ctx, data, len);
		 RIPEMD160_Final( m_Hash, &ctx);
	}

	void CRipeMd160::Hash(const char* data, int len)
	{
		 RIPEMD160_CTX ctx;
		 RIPEMD160_Init(&ctx);
		 RIPEMD160_Update(&ctx, data, len);
		 RIPEMD160_Final( m_Hash, &ctx);
	}

	void CRipeMd160::Open()
	{
		RIPEMD160_Init(&m_ctx);
	}
	void CRipeMd160::Close()
	{
		RIPEMD160_Final( m_Hash, &m_ctx);
	}
	void CRipeMd160::Update(const char* data, int len)
	{
		RIPEMD160_Update(&m_ctx, data, len);
	}
}

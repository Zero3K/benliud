/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#pragma once
#include "HashBase.h"
#include <openssl/ripemd.h>
namespace HashLib 
{
	class CRipeMd160 : public CHashBase
	{
	public:
		CRipeMd160(void);
		CRipeMd160(const char* data, int len);
		void Hash(const char* data, int len);
		int  GetHashLen() {return 20;}
		void Open();
		void Close();
		void Update(const char* data, int len);
	protected:
		RIPEMD160_CTX m_ctx;
	};
}

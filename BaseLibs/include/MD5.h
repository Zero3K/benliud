/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#pragma once

#include "HashBase.h"
#include <openssl/md5.h>
namespace HashLib
{
	class CMD5 : public CHashBase
	{
	public:
		CMD5(void);
		CMD5(const char* data, int len);
		~CMD5(void) {};
		void Hash(const char* data, int len);
		int  GetHashLen() {return 16;}
		void Open();
		void Close();
		void Update(const char* data, int len);
	protected:
		MD5_CTX m_ctx;
	};
}
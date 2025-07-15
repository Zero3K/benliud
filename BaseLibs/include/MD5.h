/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/
#pragma once

#include "HashBase.h"
#include "../../thirdparty/crypto/md5.h"
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
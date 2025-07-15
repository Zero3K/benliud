/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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

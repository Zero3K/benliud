/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/
#pragma once

#include "HashBase.h"
#include "../../thirdparty/crypto/sha1.h"
namespace HashLib
{
	class CSHA1 : public CHashBase
	{
	public:
		CSHA1(void);
		CSHA1(const char* data, int len);
		~CSHA1() {};
		void Hash(const char* data, int len);
		int  GetHashLen() {return 20;}
		void Open();
		void Close();
		void Update(const char* data, int len);

	protected:
		SHA_CTX m_ctx;
	};
}

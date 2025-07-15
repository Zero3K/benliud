/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/

#pragma once
#include "HashBase.h"
// Note: RipeMD160 implementation disabled - using stub for compatibility
// #include <openssl/ripemd.h>
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
		// Stub implementation - RipeMD160 functionality disabled
		struct RipeMD160_CTX_Stub {
			unsigned char buffer[64];
			unsigned int state[5];
			unsigned long long count;
		} m_ctx;
	};
}

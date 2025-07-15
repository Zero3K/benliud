/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

****************************************************************/
//#include "StdAfx.h"
#include "../include/RipeMd160.h"
#include <memory.h>

namespace HashLib
{
	CRipeMd160::CRipeMd160(void)
	{
		memset(&m_ctx, 0, sizeof(m_ctx));
	}

	CRipeMd160::CRipeMd160(const char* data, int len)
	{
		// Stub implementation - RipeMD160 functionality disabled
		memset(m_Hash, 0, 20);
		memset(&m_ctx, 0, sizeof(m_ctx));
	}

	void CRipeMd160::Hash(const char* data, int len)
	{
		// Stub implementation - RipeMD160 functionality disabled
		memset(m_Hash, 0, 20);
	}

	void CRipeMd160::Open()
	{
		memset(&m_ctx, 0, sizeof(m_ctx));
	}
	
	void CRipeMd160::Close()
	{
		memset(m_Hash, 0, 20);
	}
	
	void CRipeMd160::Update(const char* data, int len)
	{
		// Stub implementation - RipeMD160 functionality disabled
	}
}

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

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
		m_ctx.add(data, len);
		m_ctx.getHash(m_Hash);
	}

	void CSHA1::Hash(const char* data, int len)
	{
		m_ctx.reset();
		m_ctx.add(data, len);
		m_ctx.getHash(m_Hash);
	}

	void CSHA1::Open()
	{
		m_ctx.reset();
	}
	void CSHA1::Close()
	{
		m_ctx.getHash(m_Hash);
	}
	void CSHA1::Update(const char* data, int len)
	{
		m_ctx.add(data, len);
	}
}

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢������Э�鷢����δ���������ɲ���Ӧ�����κ���ҵ������

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
		m_ctx.add(data, len);
		m_ctx.getHash(m_Hash);
	}

	void CMD5::Hash(const char* data, int len)
	{
		m_ctx.reset();
		m_ctx.add(data, len);
		m_ctx.getHash(m_Hash);
	}

	void CMD5::Open()
	{
		m_ctx.reset();
	}

	void CMD5::Close()
	{
		m_ctx.getHash(m_Hash);
	}
	void CMD5::Update(const char* data, int len)
	{
		m_ctx.add(data, len);
	}
}

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

#pragma once

namespace HashLib
{
	class CHashBase
	{
	public:
		virtual void Open()=0;
		virtual void Close()=0;
		virtual void Update(const char* data, int len)=0;
		virtual void Hash(const char* data, int len)=0; //one time
		virtual int  GetHashLen()=0;
		virtual unsigned char* GetHash() {return m_Hash;}
	protected:
		unsigned char m_Hash[40];	//make it enough for future
	};
}

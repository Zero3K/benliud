/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

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

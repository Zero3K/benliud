/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _STREAMDATAACCEPTOR_H_
#define _STREAMDATAACCEPTOR_H_

namespace SockLib
{
	class CTCPClientSock;
	class CStreamDataAcceptor
	{
	public:
		virtual void DataNotice(CTCPClientSock* sock,int length, const char* data)=0;	//when got data 
		virtual void CloseNotice(CTCPClientSock* sock)=0;	//connection close notice
		virtual void OpenNotice(CTCPClientSock* sock)=0;	//connection build notice
		virtual void FailNotice(CTCPClientSock* sock)=0;	//connect fail or other fails
	protected:

	private:

	};
}

#endif // _STREAMDATAACCEPTOR_H_

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/HttpDataAcceptor.h"
namespace SockLib
{
	CHttpDataAcceptor::CHttpDataAcceptor(void)
	{
	}

	CHttpDataAcceptor::~CHttpDataAcceptor(void)
	{
	}

	//the sock should be CHttpSock
	void CHttpDataAcceptor::DataNotice(CTCPClientSock* sock,int length, const char* data)	//when got data 
	{
	}

	void CHttpDataAcceptor::CloseNotice(CTCPClientSock* sock)	//connection close notice
	{
	}

	void CHttpDataAcceptor::OpenNotice(CTCPClientSock* sock)	//connection build notice
	{
	}

	void CHttpDataAcceptor::FailNotice(CTCPClientSock* sock)	//connect fail or other fails
	{
	}
}
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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
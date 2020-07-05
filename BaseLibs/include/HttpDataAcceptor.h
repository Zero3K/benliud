/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#pragma once

#ifdef WIN32
#include <winsock2.h>
#endif

#ifndef llong
#ifdef WIN32
typedef __int64 llong ;
#else
typedef long long llong ;
#endif
#endif

#include <map>
#include <string>
#include "StreamDataAcceptor.h"
#include "HttpSock.h"

namespace SockLib
{
	class CHttpDataAcceptor : public CStreamDataAcceptor
	{
		class _HttpData
		{
		public:
			_HttpData() {nRetCode=0;nCurrentPos=-1;nContentLen=-1;}

			int	nRetCode;	//HTTP Status code like 200, 301, 500 etc, set to 0 when not got header
			std::string sLocation;	//for redirection
			std::string sCookie;	//the cookies
			std::string sContentType;		//content type
			llong	nContentLen;	//content length, -1 for unknown length
			llong	nCurrentPos;	//current content pos, -1 is in header
		};

	public:
		CHttpDataAcceptor(void);
		~CHttpDataAcceptor(void);
		virtual void DataNotice(CTCPClientSock* sock,int length, const char* data);	//when got data 
		virtual void CloseNotice(CTCPClientSock* sock);	//connection close notice
		virtual void OpenNotice(CTCPClientSock* sock);	//connection build notice
		virtual void FailNotice(CTCPClientSock* sock);	//connect fail or other fails
	protected:
		std::map<CHttpSock*, _HttpData> m_ClientMap;
	};
}
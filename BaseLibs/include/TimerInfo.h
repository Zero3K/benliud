

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _TIMERINFO_H
#define _TIMERINFO_H



namespace SockLib
{	
	class CTimerClient;
	class TTimerInfo
	{
	public:
		unsigned int id;
		unsigned int interval;
		unsigned int lastshot;
		bool		oneshot;
		CTimerClient* client;

	};
}

#endif




/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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


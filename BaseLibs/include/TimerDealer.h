/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/
#ifndef _TIMERDEALER_H
#define _TIMERDEALER_H

#include <list>
#include <vector>
#include "TimerInfo.h"
#include "Mutex.h"





namespace SockLib
{
	class CTimerClient;
	typedef std::list<TTimerInfo> TTimerList;
	class CTimerDealer 
	{
	public:
		virtual void RemoveTimerClient(CTimerClient* client);
		void RemoveTimer(unsigned int id);
		unsigned int AddTimer(CTimerClient* client, unsigned int interval, bool oneshot=false);
		CTimerDealer();
		virtual ~CTimerDealer();
		void DispatchTimer();

	protected:
		void CleanClient();

		unsigned int m_nTimerCounter;

		SockLib::CMutex m_PendingListMutex;

		SockLib::TTimerList m_TimerList;

		SockLib::TTimerList m_PendingList;

	};
}
#endif 

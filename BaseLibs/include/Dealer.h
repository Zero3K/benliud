/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/
#ifndef _DEALER_H_
#define _DEALER_H_

#include "TimerDealer.h"
#include "SockDealer.h"


namespace SockLib
{
	class CSock;
	class CDealer : public CSockDealer, public CTimerDealer
	{
	public:
		void Dispatch(){DispatchEvent();DispatchTimer();};
	protected:

	private:

	};
}
#endif // _DEALER_H_

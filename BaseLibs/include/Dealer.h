/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

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

/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _TIMERCLIENT_H_
#define _TIMERCLIENT_H_

namespace SockLib
{
	class CTimerClient
	{
	public:
		virtual void OnTimer( unsigned int id ) = 0;

	protected:

	private:

	};
}
#endif // _TIMERCLIENT_H_

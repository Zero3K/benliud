/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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

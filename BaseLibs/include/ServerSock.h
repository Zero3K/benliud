/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#ifndef _SERVERSOCK_H_
#define _SERVERSOCK_H_

#include "Sock.h"
namespace SockLib
{
	class CServerSock: public CSock 
	{
	public:

		virtual bool Bind(TInetAddr4 addr, bool reuse=true) 
		{
			return Bind(addr.iip,addr.iport,reuse);
		}

		virtual bool Bind( unsigned int iip, unsigned short iport , bool reuse=true)
		{
			struct sockaddr_in addr;
			memset( &addr, 0, sizeof( addr ) );

			addr.sin_family = AF_INET;
			addr.sin_port = iport;

			if ( iip != 0 )
				addr.sin_addr.s_addr = iip;
			else
				addr.sin_addr.s_addr = INADDR_ANY;

			if(reuse)
			{
#ifdef WIN32	
				bool opt=1;
				if(0!=setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR, (const char*)(&opt), sizeof(opt)))
				{
					printf("reuseaddr fail\n");
				}
#else
				int opt=1;
				if(0!=setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR, &opt,sizeof(opt)))
				{
					printf("reuseaddr fail\n");
				}
#endif	
			}

			return 0 == bind( m_hSocket, ( struct sockaddr* ) & addr, sizeof( struct sockaddr ) ) ;
		}
	protected:

	private:

	};
}
#endif // _SERVERSOCK_H_

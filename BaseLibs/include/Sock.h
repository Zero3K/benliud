/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#ifndef _SOCK_H_
#define _SOCK_H_

#include "TimerClient.h"
#include "SockDealer.h"

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#endif

#include <string>
#include <assert.h>

namespace SockLib
{

	class TInetAddr4
	{
	public:
		TInetAddr4(){};
		TInetAddr4(unsigned int a,unsigned short b)
		{
			iip=a;
			iport=b;
		}

		bool operator==(const TInetAddr4& other) const {
			return iip==other.iip && iport==other.iport;
		};	

		void operator=(const TInetAddr4& other) {
			iip=other.iip;
			iport=other.iport;
		};

		unsigned int iip;
		unsigned short iport;
	};

	class TInetAddr6
	{
	public:
		TInetAddr6(){};
		TInetAddr6(unsigned char* a,unsigned short b)
		{
			ipv6=a;
			iport=b;
		}

		bool operator==(const TInetAddr6& other) const {
			return ipv6==other.ipv6 && iport==other.iport;
		};	

		void operator=(const TInetAddr6& other) {
			ipv6=other.ipv6;
			iport=other.iport;
		};

		class _ipv6
		{
		public:
			_ipv6() {};
			_ipv6(unsigned char* a) {memcpy(ipdata, a, 16);}
			void operator=(unsigned char* a) {memcpy(ipdata, a, 16);}
			void operator=(const _ipv6& other) {memcpy(ipdata, other.ipdata, 16);} 
			bool operator==(const _ipv6& other) const {
				return memcmp(ipdata, other.ipdata, 16)==0;
			}
			unsigned char ipdata[16];
		}ipv6;

		unsigned short iport;
	};

	class CDealer;
	class CSock: public CTimerClient
	{
	public:
		void SetLinger(bool linger);
		CSock():m_hSocket(-1),m_pDealer(NULL),m_bMaskRead(false),m_bMaskWrite(false){};
		~CSock() { Close();}
		int GetHandle() { return m_hSocket; }
		CDealer* GetDealer() { return m_pDealer;}
		void SetDealer( CDealer* dealer ) ;
		unsigned int AddTimer(unsigned int interval, bool oneshot=false);
		void RemoveTimer(unsigned int id);
		void Attach(int fd);
		virtual void Close();

		void maskRead( bool mask )
		{
			m_bMaskRead = mask;
		}

		void maskWrite( bool mask )
		{
			m_bMaskWrite = mask;

		}

		bool maskRead()
		{
			return m_bMaskRead;
		}

		bool maskWrite()
		{
			return m_bMaskWrite;
		}

		bool SetBlock(bool block)
		{
			if(!block)
			{//nonblock
				if(m_hSocket==-1) return false;

#ifdef WIN32


				u_long iMode = 1;
				ioctlsocket( m_hSocket, FIONBIO, &iMode );

#else

				int flags;
				flags = fcntl( m_hSocket, F_GETFL, 0 );

				if(flags < 0) return false;

				flags |= O_NONBLOCK;

				if ( fcntl( m_hSocket, F_SETFL, flags ) < 0 )
				{
					return false;
				}


#endif
				return true;
			}
			else
			{//block
				if(m_hSocket==-1) return false;

#ifdef WIN32

				u_long iMode = 0;
				ioctlsocket( m_hSocket, FIONBIO, &iMode );

#else

				int flags;
				flags = fcntl( m_hSocket, F_GETFL, 0 );

				if(flags < 0) return false;

				flags &= ~O_NONBLOCK;

				if ( fcntl( m_hSocket, F_SETFL, flags ) < 0 )
				{
					return false;
				}


#endif	
				return true;
			}
		}

		virtual bool CreateSock()=0;
		virtual void OnRead(){};
		virtual void OnWrite(){};
		virtual void OnTimer(unsigned int){};


	protected:
		CDealer* m_pDealer;
		int		m_hSocket;
		bool	m_bMaskRead;
		bool	m_bMaskWrite;	
	private:

	};

}
#endif // _SOCK_H_

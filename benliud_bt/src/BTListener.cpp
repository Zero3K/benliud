/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/

#include "stdafx.h"

// BTListener.cpp: implementation of the CBTListener class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif
#include "../include/BTListener.h"
#include <Dealer.h>
#include <AutoLock.h>
#include "../include/BTListenSocket.h"
#include "../include/BTPeer.h"
#include "../include/PeerAdminBase.h"
#include "../include/BTListenPeerAdmin.h"
#include "../include/SpeedControlBase.h"
#include "../include/BTListenSpeedControl.h"
#include "../include/BTStorage.h"
#include <SHA1.h>

#include <assert.h>

extern void syslog( std::string info );
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBTListener::CBTListener()
{
	m_bStop=false;
	m_pDealer=NULL;

	m_pListenSocket=NULL;
	m_pListenPeerAdmin=NULL;
	m_pListenSpeedControl=NULL;

	m_nGlobalUpByteCount=0x0FFFFFFF;
	m_nGlobalDownByteCount=0x0FFFFFFF;
	m_nGlobalUploadSpeedLimit=0x0FFFFFFF;
	m_nGlobalDownloadSpeedLimit=0x0FFFFFFF;
}

CBTListener::~CBTListener()
{
	if(m_pDealer)
		delete m_pDealer;
	if(m_pListenSocket)
		delete m_pListenSocket;
	if(m_pListenPeerAdmin)
		delete m_pListenPeerAdmin;
	if(m_pListenSpeedControl)
		delete m_pListenSpeedControl;
}

bool CBTListener::Start(unsigned int port)
{


	m_nListenPort=port;

	m_pDealer=new SockLib::CDealer();

	m_pListenPeerAdmin=new CBTListenPeerAdmin(this);
	m_pListenSpeedControl=new CBTListenSpeedControl;
	
	m_pListenPeerAdmin->Start();

	m_bStop=false;
	return Run( false ); //a joinable thread start
}

void CBTListener::Stop()
{

	m_bStop=true;
	Wait();

	m_pListenPeerAdmin->Stop();

	delete m_pListenSpeedControl;
	m_pListenSpeedControl=NULL;
	delete m_pListenPeerAdmin;
	m_pListenPeerAdmin=NULL;

	delete m_pDealer;
	m_pDealer=NULL;

}

void CBTListener::Entry()
{
#ifdef WIN32
    WSADATA wsadata;
    WSAStartup( MAKEWORD( 2, 0 ), &wsadata );
#endif


	m_pListenSocket=new CBTListenSocket(this);
	m_pListenSocket->SetDealer(m_pDealer);
	m_pListenSocket->Start(m_nListenPort);


	unsigned int lasttick=GetTickCount();


	while(!m_bStop)
	{
		unsigned int thistick = GetTickCount();
		
		if ( thistick - lasttick >= 500 )
		{

			m_nGlobalUpByteCount += int((float(thistick - lasttick) / 1000.0f)* m_nGlobalUploadSpeedLimit);
			if(m_nGlobalUpByteCount > m_nGlobalUploadSpeedLimit) m_nGlobalUpByteCount=m_nGlobalUploadSpeedLimit;

			m_nGlobalDownByteCount += int((float(thistick - lasttick) / 1000.0f)* m_nGlobalDownloadSpeedLimit);
			if(m_nGlobalDownByteCount > m_nGlobalDownloadSpeedLimit) m_nGlobalDownByteCount=m_nGlobalDownloadSpeedLimit;

			lasttick = thistick;
		}

		m_pDealer->Dispatch();
		m_pListenSpeedControl->Update();

	}


	m_pListenSocket->Stop();

	delete m_pListenSocket;
	m_pListenSocket=NULL;


#ifdef WIN32
    //WSACleanup();
#endif
}

SockLib::CDealer* CBTListener::GetDealer()
{
	return m_pDealer;
}


CSpeedControlBase* CBTListener::GetSpeedControl()
{
	return m_pListenSpeedControl;
}


//�����棬����������Ѿ���ת��
//���ؼ٣�����������Ҫ�ر�
bool CBTListener::LinkGotHash(std::string hash,CBTPeer* peer)
{
	TJobMap::const_iterator it;
	
	SockLib::CAutoLock al(m_JobMapMutex);

	it=m_JobMap.find(hash);

	if(it==m_JobMap.end())
	{
		return false;
	}
	else
	{
		try
		{
			return it->second->TransferPeer(peer);
		}
		catch(...)
		{
			return false;
		}
	}

	return false;
}

void CBTListener::RegisteTask(std::string hash, CBTStorage *task)
{
	SockLib::CAutoLock al(m_JobMapMutex);
	m_JobMap[hash]=task;

}

void CBTListener::UnregisteTask(std::string hash)
{

	SockLib::CAutoLock al(m_JobMapMutex);
	m_JobMap.erase(hash);


}

unsigned int CBTListener::GetListenPort()
{
	return m_nListenPort;
}



bool CBTListener::LinkGotEncryptHash(std::string hashxor, MSE::BigInt S, CBTPeer* peer)
{

	//hashxor=HASH('req2', infohash) xor HASH('req3', S)  
	TJobMap::const_iterator it;
	
	SockLib::CAutoLock al(m_JobMapMutex);

	for(it=m_JobMap.begin();it!=m_JobMap.end();it++)
	{
		if(HashXor(it->first, S)==hashxor)
		{
			try
			{
				bool ok=it->second->TransferPeer(peer);
				return ok;
			}
			catch(...)
			{
				assert(false);
				return false;
			}
		}
	}

	return false;

}

std::string CBTListener::HashXor(std::string hash, MSE::BigInt S)
{
	//HASH('req2', infohash) xor HASH('req3', S)
	
	unsigned char hash2[20];
	unsigned char hash3[20];

	unsigned char req2[100];
	unsigned char req3[100];

	memcpy(req2,"req2",4);
	memcpy(req2+4, hash.data(),20);

	memcpy(req3,"req3",4);
	S.toBuffer(req3+4,96);

	HashLib::CSHA1 tmp;
	tmp.Hash((const char*)req2,24);
	memcpy(hash2, tmp.GetHash(), tmp.GetHashLen());
	//SHA1Block(req2,24,hash2);

	tmp.Hash((const char*)req3, 100);
	memcpy(hash3, tmp.GetHash(), tmp.GetHashLen());
	//SHA1Block(req3,100,hash3);

	for(int i=0;i<20;i++)
	{
		hash2[i]^=hash3[i];
	}

	std::string ret;
	ret.append((const char*)hash2,20);
	return ret;
}

int CBTListener::RunOffDownBytes(int bytes)
{
	//CAutoLock al(m_DownloadSpeedMutex);
	SockLib::CAutoLock al(m_nGlobalDownByteCountMutex);
	m_nGlobalDownByteCount-=bytes;   //����ԭ�Ӳ��������������ܿ����ٶȲ���ô׼ȷ
	return m_nGlobalDownByteCount;
}

int CBTListener::RunOffUpBytes(int bytes)
{
	//CAutoLock al(m_UploadSpeedMutex);
	SockLib::CAutoLock al(m_nGlobalUpByteCountMutex);
	m_nGlobalUpByteCount-=bytes;	//����ԭ�Ӳ��������������ܿ����ٶȲ���ô׼ȷ
	return m_nGlobalUpByteCount;
}

int CBTListener::GetLeftUpBytes()
{
	//SockLib::CAutoLock al(m_nGlobalUpByteCountMutex);
	return m_nGlobalUpByteCount;
}

int CBTListener::GetLeftDownBytes()
{
	//SockLib::CAutoLock al(m_nGlobalDownByteCountMutex);
	return m_nGlobalDownByteCount;
}

void CBTListener::SetUploadSpeedLimit(int speed)
{
	m_nGlobalUploadSpeedLimit=speed;
	if(speed==0) m_nGlobalUploadSpeedLimit=0x0FFFFFFF;
}

void CBTListener::SetDownloadSpeedLimit(int speed)
{
	m_nGlobalDownloadSpeedLimit=speed;
	if(speed==0) m_nGlobalDownloadSpeedLimit=0x0FFFFFFF;
}

void CBTListener::NewAccept(int handle, unsigned int iip, unsigned short iport)
{
	if(!m_pListenPeerAdmin->NewAccept(handle,iip,iport))
	{

#ifdef WIN32
		closesocket(handle);
#else
		close(handle);
#endif
	}
}

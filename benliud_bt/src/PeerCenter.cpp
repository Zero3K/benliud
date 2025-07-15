/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

// PeerCenter.cpp: implementation of the CPeerCenter class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include <vector>
#include "../include/PeerCenter.h"

#include <AutoLock.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void syslog(std::string info);

CPeerCenter::CPeerCenter()
{
	m_nConnection=0;	//
	m_nMyInitConnection=0;	//Ƿγɵ
	m_nPeerInitConnection=0;	//Էγɵ

	m_nConnecting=0;	//
	m_nTotalPeer=0;	//
	m_nConnectingLimit=20;
	m_nConnectionLimit=50;
	m_UploadMode=false;

	m_nTaskId=0;
	m_nLastCalSpeedTick=0;

	for(int i=0;i<SESSIONNUM;i++)
	{
		m_nSessionConnecting[i]=0;
		m_nSessionConnection[i]=0;
	}


}

CPeerCenter::~CPeerCenter()
{
}

//trackerӿõĻ
bool CPeerCenter::AddAvialablePeerInfo(unsigned int iip, unsigned short iport)
{
	SockLib::CAutoLock al(m_Mutex);

	unsigned int haship=HashIP(iip);	//ʹHashIPΪ˴IPԣͬʱһIPη

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(haship);

	if(it!=m_PeerInfoMap.end()) 
	{
		if(it->second.m_iPort!=iport)
		{
			it->second.m_iPort=iport; //¶˿ݴ
			//it->second.m_FailCount=0;
			//it->second.m_SuccCount=0;
			it->second.m_Connective=0;
			it->second.m_NextTry=0;
		}
		return false;
	}

	TPeerDetail newpeer(iip, iport);
	
	m_PeerInfoMap[haship]=newpeer;

	m_nTotalPeer++;

	return true;
}

//һȥִ
//connectable ǷһѾȷΪӵĽڵ㣿2007/12/30 
bool CPeerCenter::GetPeerInfoToLink(unsigned int sessid, bool connectable, unsigned int& iip, unsigned short& iport, int& encref, unsigned int& timeout)
{

	assert(sessid < SESSIONNUM);

	SockLib::CAutoLock al(m_Mutex);

	if(m_nConnecting >= m_nConnectingLimit) return false; //ܵӷ
//	if(m_nConnection >= m_nConnectionLimit) return false; //ܵ

	//ҪƣPeerAdminƣʱ᳢ԹرЧ
//	if(m_nConnected  >= m_nConnectionLimit) return false; //ܵ

	//ڵsession, Զһ1010/3=3ſһ
	//ôsessionֻܷ9ӣﲻ >=
	if(m_nSessionConnecting[sessid] > m_nConnectingLimit/SESSIONNUM) return false;
//	if(m_nSessionConnection[sessid] > m_nConnectionLimit/SESSIONNUM) return false;

	//°취ѡӵģȼ
	unsigned int now=GetTickCount();

	int priority=-1;
	//*/
	unsigned int haship;
	TPeerInfoMap::iterator it;

	for(it=m_PeerInfoMap.begin();it!=m_PeerInfoMap.end();it++)
	{
		if(it->second.AvialableForLink(sessid,connectable,now))
		{
			int prio=it->second.LinkPriority(sessid);

			if(prio > priority)
			{
				haship=it->first;
				priority=prio;
			}
		}
	}

	if(priority==-1) return false;

/*/
	std::vector<unsigned int> haships;

	TPeerInfoMap::iterator it;

	for(it=m_PeerInfoMap.begin();it!=m_PeerInfoMap.end();it++)
	{
		if(it->second.AvialableForLink(sessid,connectable,now))
		{
			int prio=it->second.LinkPriority(sessid);

			if(prio > priority)
			{
				haships.clear();
				haships.push_back(it->first);
				priority=prio;

			}
			else if(prio==priority)
			{
				haships.push_back(it->first);
				
			}
		}
	}

	if(priority==-1) return false;
	if(haships.empty()) return false;

	unsigned int randpos=rand()%haships.size();
	unsigned int haship=haships[randpos];
/*/
	m_PeerInfoMap[haship].RecordLinking(sessid);

	iip=m_PeerInfoMap[haship].m_iIP;
	iport=m_PeerInfoMap[haship].m_iPort;
	encref=m_PeerInfoMap[haship].m_EncryptRef;
	timeout=m_PeerInfoMap[haship].m_nTimeOut;

	m_nConnecting++;
	
	m_nSessionConnecting[sessid]++;

	return true;


}

//ʧܺӳɹҪͨΪҪ¼״̬
void CPeerCenter::LinkReport(unsigned int sessid, unsigned int iip, bool ok)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.RecordLinkResult(sessid,ok);

	m_nConnecting--;
	m_nSessionConnecting[sessid]--;
	
	if(!ok) return;
	
	
	m_nMyInitConnection++;
	m_nConnection++;
	m_nSessionConnection[sessid]++;
/*	
	if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
	{
		//ˢ¶Էȵʾ,ʱȫˢ·ʽ
		char arg[42];
		
		//link8λ֣ڱʾͽ
		unsigned int link=((it->second.m_OutLinkCount)<<4)+(it->second.m_InLinkCount);
		float prog=it->second.GetPeerProgress();
		unsigned int dspd=0;
		unsigned int uspd=0;
		
		memcpy(arg+0, &(it->second.m_iIP), 4);
		memcpy(arg+4, &(link), 4);
		memcpy(arg+8, &(prog), 4);
		memcpy(arg+12, &(it->second.m_EncryptRef), 4);
		memcpy(arg+16, &(it->second.m_ByteGot), 4);
		memcpy(arg+20, &(it->second.m_ByteSend), 4);
		memcpy(arg+24, &dspd, 4);
		memcpy(arg+28, &uspd, 4);
		memcpy(arg+32, it->second.m_PeerId.data(), 10);
		
		m_pTaskEvent(m_nTaskId, _PEER_FULLLIST, 42, 0, (void*)&arg);
	}
*/	
	
}

//رӵı棬ȱӳɹĲҪʧܵĲñ棬linkreport漴
//reason: close reason
//dsum: download sum
//usum: upload sum
//bitpiece: ԷеƬԹ۲쵽ԷǷƭΪ
void CPeerCenter::CloseReport(unsigned int sessid, unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	m_nConnection--;
	m_nSessionConnection[sessid]--;

	if(accepted) {
		m_nPeerInitConnection--;
	}
	else {
		m_nMyInitConnection--;
	}

	//ܹرյǽӣǿܻûݣ޸ĺ÷ˣȽĿ
	if(it==m_PeerInfoMap.end()) {
		return; 
	}

	it->second.RecordClose(sessid,reason,accepted,peerid,bitset);
/*
	if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
	{
		//ˢ¶Էȵʾ,ʱȫˢ·ʽ
		char arg[42];

		//link8λ֣ڱʾͽ
		unsigned int link=((it->second.m_OutLinkCount)<<4)+(it->second.m_InLinkCount);
		float prog=it->second.GetPeerProgress();
		unsigned int dspd=0;
		unsigned int uspd=0;

		memcpy(arg+0, &(it->second.m_iIP), 4);
		memcpy(arg+4, &(link), 4);
		memcpy(arg+8, &(prog), 4);
		memcpy(arg+12, &(it->second.m_EncryptRef), 4);
		memcpy(arg+16, &(it->second.m_ByteGot), 4);
		memcpy(arg+20, &(it->second.m_ByteSend), 4);
		memcpy(arg+24, &dspd, 4);
		memcpy(arg+28, &uspd, 4);
		memcpy(arg+32, it->second.m_PeerId.data(), 10);

		m_pTaskEvent(m_nTaskId, _PEER_FULLLIST, 42, 0, (void*)&arg);
	}
*/
}

//sessionԽʱѯǷܣ
//ﷵΪѾ¼˽
bool CPeerCenter::TryAcceptPeerLink(unsigned int sessid, unsigned int iip)
{
	if(m_nConnection  >= m_nConnectionLimit) return false; //ܵ

	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) 
	{//ΪȱʧIPĿ,ûݣ¼һҪ

		TPeerDetail newpeer(iip,0);

		newpeer.TryAccept(sessid); //϶
		m_PeerInfoMap[HashIP(iip)]=newpeer;

		m_nTotalPeer++;
		m_nPeerInitConnection++;
		m_nConnection++;

		m_nSessionConnection[sessid]++;

/*
		if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
		{
			//ˢ¶Էȵʾ,ʱȫˢ·ʽ
			char arg[42];

			it=m_PeerInfoMap.find(HashIP(iip));

			//link8λ֣ڱʾͽ
			unsigned int link=((it->second.m_OutLinkCount)<<4)+(it->second.m_InLinkCount);
			float prog=it->second.GetPeerProgress();
			unsigned int dspd=0;
			unsigned int uspd=0;

			memcpy(arg+0, &(it->second.m_iIP), 4);
			memcpy(arg+4, &(link), 4);
			memcpy(arg+8, &(prog), 4);
			memcpy(arg+12, &(it->second.m_EncryptRef), 4);
			memcpy(arg+16, &(it->second.m_ByteGot), 4);
			memcpy(arg+20, &(it->second.m_ByteSend), 4);
			memcpy(arg+24, &dspd, 4);
			memcpy(arg+28, &uspd, 4);
			memcpy(arg+32, it->second.m_PeerId.data(), 10);

			m_pTaskEvent(m_nTaskId, _PEER_FULLLIST, 42, 0, (void*)&arg);
		}
*/
		return true; 
	}

	bool check=it->second.TryAccept(sessid);

	if(!check) return false;

	m_nPeerInitConnection++;
	m_nConnection++;
	m_nSessionConnection[sessid]++;
/*
	if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
	{
		//ˢ¶Էȵʾ,ʱȫˢ·ʽ
		char arg[42];

		//link8λ֣ڱʾͽ
		unsigned int link=((it->second.m_OutLinkCount)<<4)+(it->second.m_InLinkCount);
		float prog=it->second.GetPeerProgress();
		unsigned int dspd=0;
		unsigned int uspd=0;

		memcpy(arg+0, &(it->second.m_iIP), 4);
		memcpy(arg+4, &(link), 4);
		memcpy(arg+8, &(prog), 4);
		memcpy(arg+12, &(it->second.m_EncryptRef), 4);
		memcpy(arg+16, &(it->second.m_ByteGot), 4);
		memcpy(arg+20, &(it->second.m_ByteSend), 4);
		memcpy(arg+24, &dspd, 4);
		memcpy(arg+28, &uspd, 4);
		memcpy(arg+32, it->second.m_PeerId.data(), 10);

		m_pTaskEvent(m_nTaskId, _PEER_FULLLIST, 42, 0, (void*)&arg);
	}
*/
	return true;

}

void CPeerCenter::SetConnectingLimit(unsigned int limit)
{
	m_nConnectingLimit=limit;
}

void CPeerCenter::SetConnectionLimit(unsigned int limit)
{
	m_nConnectionLimit=limit;
}

unsigned int CPeerCenter::GetTotalPeer()
{
	return m_nTotalPeer;
}

unsigned int CPeerCenter::GetTotalConnected()
{
	return m_nConnection;
}

unsigned int CPeerCenter::GetMyInitConnected()
{
	return m_nMyInitConnection;
}

unsigned int CPeerCenter::GetPeerInitConnected()
{
	return m_nPeerInitConnection;
}

unsigned int CPeerCenter::GetConnecting()
{
	return m_nConnecting;
}

void CPeerCenter::DownloadFinish(bool finish)
{
	if(!m_UploadMode && finish)
	{
		m_UploadMode=true;
		//reset all the data
		SockLib::CAutoLock al(m_Mutex);

		TPeerInfoMap::iterator it;
		for(it=m_PeerInfoMap.begin();it!=m_PeerInfoMap.end();it++)
		{
			it->second.ResetWhenFinish();

		}

	}
	else
	{
		m_UploadMode=finish;
	}

}

void CPeerCenter::AddAvialableBenliudPeerInfo(unsigned int iip, unsigned short iport)
{
	AddAvialablePeerInfo(iip,iport);
}

//ԴƷ
void CPeerCenter::GiveUpLink(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.GiveUpLink(sessid);
	m_nSessionConnecting[sessid]--;
	m_nConnecting--;

}

//bitset, ϵļ¼٣˵ƭ͵ģӦ
//ϵļ¼滻ϵļ¼bitsetλӦǵ
int CPeerCenter::CheckBitSet(unsigned int sessid, std::string& peerid, unsigned int iip, CBTPiece& bitset)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) 
	{
		//ûҵIPһµַĿ˿Ϊ0
		TPeerDetail newpeer(iip,0);
		newpeer.CheckBitSet(sessid,peerid,bitset); //϶棬¼bitset
		m_PeerInfoMap[HashIP(iip)]=newpeer;
		m_nTotalPeer++;
		return 0;
	}

	int nret=it->second.CheckBitSet(sessid,peerid,bitset);

	return nret;
}


unsigned int CPeerCenter::HashIP(unsigned int iip)
{
//	return 0xAAAAAAAA ^ ip; //0xAA = 10101010
//	return (iip << 16) + (iip >> 16);

	return 
	((iip & 0xFF000000) >> 24) +
	((iip & 0x000000FF) << 8 ) +
	((iip & 0x0000FF00) << 16) +
	((iip & 0x00FF0000));
}

void CPeerCenter::LinkOkButNoRoomClose(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.RecordLinkOkNoRoomClose(sessid);  //ȱ

	m_nConnecting--;
	m_nSessionConnecting[sessid]--;

}

//TryAcceptPeerLink෴
void CPeerCenter::GiveUpAcceptPeerLink(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip)); //϶ҵ

	it->second.GiveUpAcceptPeerLink(sessid);

}

void CPeerCenter::LinkOkButPeerClose(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.RecordLinkOkPeerClose(sessid);  //ȱ

	m_nConnecting--;
	m_nSessionConnecting[sessid]--;
}
 
bool CPeerCenter::AnyUnCheckedNode()
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;
	for(it=m_PeerInfoMap.begin();it!=m_PeerInfoMap.end();it++)
	{
		if(it->second.UnCheckedNode()) return true;
	}
 
	return false;
} 

void CPeerCenter::PeerSupportEncryption(unsigned int iip, bool enc)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.PeerSupportEncryption(enc); 

}

//ͳ
void CPeerCenter::GotChunk(unsigned int iip, int chunks)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.GotChunk(chunks);  

}

//ͳ
void CPeerCenter::SendChunk(unsigned int iip, int chunks)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//ΪǲɾݣԲҲӦĿ
	it->second.SendChunk(chunks);  

}


void CPeerCenter::PeerHaveNewPieceNotice(unsigned int iip, int index)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	it->second.PeerHaveNewPiece(index);

}

int CPeerCenter::GetPeerCredit(unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	return it->second.GetCredit();

}

//ַǷԽ
bool CPeerCenter::CheckAccept(unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) return true; //

	return it->second.CheckAccept();
}

void CPeerCenter::TickForCalSpeed(void)
{

	SockLib::CAutoLock al(m_Mutex);

	unsigned int thistick=GetTickCount();
	unsigned int gap=thistick-m_nLastCalSpeedTick;

	TPeerInfoMap::iterator it;

	for(it=m_PeerInfoMap.begin();it!=m_PeerInfoMap.end();it++)
	{
		it->second.CalSpeedTick(gap);
	}

	m_nLastCalSpeedTick=thistick;
}

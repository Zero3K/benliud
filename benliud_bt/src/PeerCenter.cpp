/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


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
	m_nConnection=0;	//已连接
	m_nMyInitConnection=0;	//我们发起形成的连接数
	m_nPeerInitConnection=0;	//对方发起形成的连接数

	m_nConnecting=0;	//正连接
	m_nTotalPeer=0;	//总数
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

//tracker添加可用的伙伴数据
bool CPeerCenter::AddAvialablePeerInfo(unsigned int iip, unsigned short iport)
{
	SockLib::CAutoLock al(m_Mutex);

	unsigned int haship=HashIP(iip);	//使用HashIP是为了打乱IP的连续性，避免同时对一个IP段发起大量连接

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(haship);

	if(it!=m_PeerInfoMap.end()) 
	{
		if(it->second.m_iPort!=iport)
		{
			it->second.m_iPort=iport; //新端口数据代替老数据
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

//获得一个任务去执行连接
//connectable ：是否请求一个已经被确认为可连接的节点？2007/12/30 增加
bool CPeerCenter::GetPeerInfoToLink(unsigned int sessid, bool connectable, unsigned int& iip, unsigned short& iport, int& encref, unsigned int& timeout)
{

	assert(sessid < SESSIONNUM);

	SockLib::CAutoLock al(m_Mutex);

	if(m_nConnecting >= m_nConnectingLimit) return false; //总的连接发起限制
//	if(m_nConnection >= m_nConnectionLimit) return false; //总的连接限制

	//不要限制，PeerAdmin有限制，当超过时，会尝试关闭无效连接
//	if(m_nConnected  >= m_nConnectionLimit) return false; //总的连接限制

	//对于单个session, 可以多放一个连接数，比如总数是10，10/3=3，如果不放宽一点
	//那么三个session最多只能发出9个连接，所以这里不用 >=
	if(m_nSessionConnecting[sessid] > m_nConnectingLimit/SESSIONNUM) return false;
//	if(m_nSessionConnection[sessid] > m_nConnectionLimit/SESSIONNUM) return false;

	//新办法，选出可连接的，按优先级来排列连接
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

//连接失败和连接成功都需要通过这个报告过来，因为我们要记录状态
void CPeerCenter::LinkReport(unsigned int sessid, unsigned int iip, bool ok)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//因为我们不删除数据，所以不会找不到对应条目
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
		//刷新对方进度的显示,暂时采用全刷新方式
		char arg[42];
		
		//link后8位分两部分，用于表示发出和进来的连接数
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

//关闭连接的报告，事先报告过连接成功的才需要报告这个，连接失败的不用报告，用linkreport报告即可
//reason: close reason
//dsum: download sum
//usum: upload sum
//bitpiece: 对方有的片数，可以观察到对方是否存在欺骗行为
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

	//可能关闭的是进入的连接，而我们可能还没有这个数据，不过修改后按理不该发生这种事了，我们事先建立了项目
	if(it==m_PeerInfoMap.end()) {
		return; 
	}

	it->second.RecordClose(sessid,reason,accepted,peerid,bitset);
/*
	if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
	{
		//刷新对方进度的显示,暂时采用全刷新方式
		char arg[42];

		//link后8位分两部分，用于表示发出和进来的连接数
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

//session尝试接受连接时，调用这个询问是否接受？
//这里返回真则必须接受他，因为我们已经记录了接受连接
bool CPeerCenter::TryAcceptPeerLink(unsigned int sessid, unsigned int iip)
{
	if(m_nConnection  >= m_nConnectionLimit) return false; //总的连接限制

	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) 
	{//为缺失的IP建立项目,这个东西我们没有数据，新家伙要接受

		TPeerDetail newpeer(iip,0);

		newpeer.TryAccept(sessid); //肯定返回真
		m_PeerInfoMap[HashIP(iip)]=newpeer;

		m_nTotalPeer++;
		m_nPeerInitConnection++;
		m_nConnection++;

		m_nSessionConnection[sessid]++;

/*
		if(m_bActive && ( it->second.m_ByteGot!=0 || it->second.m_ByteSend!=0 ))
		{
			//刷新对方进度的显示,暂时采用全刷新方式
			char arg[42];

			it=m_PeerInfoMap.find(HashIP(iip));

			//link后8位分两部分，用于表示发出和进来的连接数
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
		//刷新对方进度的显示,暂时采用全刷新方式
		char arg[42];

		//link后8位分两部分，用于表示发出和进来的连接数
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

//因资源限制放弃连接任务
void CPeerCenter::GiveUpLink(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//因为我们不删除数据，所以不会找不到对应条目
	it->second.GiveUpLink(sessid);
	m_nSessionConnecting[sessid]--;
	m_nConnecting--;

}

//检查bitset, 如果比老的记录还少，说明是欺骗型的，应封闭他
//如果比老的记录大，则替换老的记录，bitset是置位的数量，正常应该是递增的
int CPeerCenter::CheckBitSet(unsigned int sessid, std::string& peerid, unsigned int iip, CBTPiece& bitset)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) 
	{
		//没有找到IP，建立一个新地址项目，端口为0，不可连接
		TPeerDetail newpeer(iip,0);
		newpeer.CheckBitSet(sessid,peerid,bitset); //肯定返回真，记录这个bitset
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
	
	//因为我们不删除数据，所以不会找不到对应条目
	it->second.RecordLinkOkNoRoomClose(sessid);  //先报告连接正常

	m_nConnecting--;
	m_nSessionConnecting[sessid]--;

}

//和TryAcceptPeerLink做相反的事
void CPeerCenter::GiveUpAcceptPeerLink(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip)); //肯定能找到

	it->second.GiveUpAcceptPeerLink(sessid);

}

void CPeerCenter::LinkOkButPeerClose(unsigned int sessid, unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//因为我们不删除数据，所以不会找不到对应条目
	it->second.RecordLinkOkPeerClose(sessid);  //先报告连接正常

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
	
	//因为我们不删除数据，所以不会找不到对应条目
	it->second.PeerSupportEncryption(enc); 

}

//用于数据流统计
void CPeerCenter::GotChunk(unsigned int iip, int chunks)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//因为我们不删除数据，所以不会找不到对应条目
	it->second.GotChunk(chunks);  

}

//用于数据流统计
void CPeerCenter::SendChunk(unsigned int iip, int chunks)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));
	
	//因为我们不删除数据，所以不会找不到对应条目
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

//检查这个地址是否可以接受他连接
bool CPeerCenter::CheckAccept(unsigned int iip)
{
	SockLib::CAutoLock al(m_Mutex);

	TPeerInfoMap::iterator it;

	it=m_PeerInfoMap.find(HashIP(iip));

	if(it==m_PeerInfoMap.end()) return true; //新连接

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

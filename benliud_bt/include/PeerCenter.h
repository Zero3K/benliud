/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// PeerCenter.h: interface for the CPeerCenter class.
//
//////////////////////////////////////////////////////////////////////

//peer数据管理中心
//以后peeradmin就统一到这里取数据

#ifndef _PEERCENTER_H
#define _PEERCENTER_H

#include <string>
#include <map>
#include <vector>


#include <Tools.h>
#include <Mutex.h>
#include "../benliud/datatype_def.h"
#include "BTPeer.h"
#include "sessiondef.h"
#include "BTPiece.h"

#ifdef _CHECK
extern void syslog(std::string info);
#endif


/*

新连接优先级别2008/01/28：

未主动连接的且没有被连接发生的，20	这是初始的状态
未主动连接的但已经被连接过的，当前连接数==0：15，争取反过去连接对方，虽然可能对方是连不到的
未主动连接的但已经被连接过的，当前连接数==1：10，争取反过去连接对方，虽然可能对方是连不到的
未主动连接的但已经被连接过的，当前连接数>=2: 5， 已经有两个连接了，不着急建立新连接

已经主动连接过的，曾经失败一次：7，
已经主动连接过的，曾经失败二次：4，如果有被对方连接则直接降低为0
已经主动连接过的，曾经失败三次以上：0，

已经主动连接过的，确认了可连接，当前发起的连接数为0的，12-n ，n为成功连接的次数
已经主动连接过的，确认了可连接，当前发起的连接数为1的，9-n , n为成功连接的次数
已经主动连接过的，确认了可连接，当前发起的连接数为>=2的，1
*/

/*
已经主动连接过的，如果没有成功过，则失败越多优先越低，失败一次：3，失败两次：2，失败三次以上1。
失败1次以上且发生过被连接，则优先为0；

如果成功连接过，而当前无连接的，优先为6，如果当前有一个连接的，优先为5，如果当前有两个以上连接的，优先为4




连接优先级排序, 连接发起的优先级不参考下在量：

没有发生连接的，最高，优先级10

连接成功过，并且当前连接数<1， 优先级7

连接成功过，并且当前连接数<2， 优先级6


连接失败一次的，禁止连接时间=1*30 ，优先级3

连接失败两次的，禁止连接时间=2*30， 优先级2

连接失败n次的，禁止连接时间=2^n*30，优先级1


连接失败一次以上，并且发现了被动连接的，禁止连接时间=3600s ，优先级0

每次发起连接后最少间隔15秒以后再尝试连接。对于第一次连接成功的，下一次
连接允许在5秒后发起，这样可以加快启动速度。

*/


class CBTPiece;


class CPeerCenter  
{

	struct _t
	{
		unsigned int	ip;
		unsigned int	link;	//连接总数
		float			prog;	//4字节，希望64位上也是4字节
		int				encref;	//4字节
		unsigned int	down;
		unsigned int	up;
		float			downspd;
		float			upspd;
		char			id[10];
	};

	class TPeerDetail
	{
	public:
		TPeerDetail(unsigned int iip=0, unsigned short port=0)
		{
			m_iIP=iip;
			m_iPort=port;
			m_Connective=0;
			m_LastClose=0;
			m_NextTry=0;
			m_EncryptRef=0;
			m_DataAmount=0;
			m_OutLinkCount=0;
			m_InLinkCount=0;
			m_LinkingRecord=0;
			m_LinkRecord=0;
			m_BanedTime=0;
			m_ByteGot=0;
			m_ByteSend=0;
			m_LastByteGot=0;
			m_LastByteSend=0;
			m_LinkByCount=0; 
			m_DownSpd=0;
			m_UpSpd=0;
			m_nTimeOut=20;	//动态的，每次连接失败+3直到60秒

			m_pBitSet=NULL;
			m_nShowTick=0;

		}

		virtual ~TPeerDetail()
		{
			if(m_pBitSet!=NULL)
				delete m_pBitSet;
		}

		//是否未检查的节点
		bool UnCheckedNode()
		{
			return m_Connective==0;
		}

		//是否可用于发起连接？
		bool AvialableForLink(unsigned int sess, bool connectable, unsigned int now) const
		{
			if(m_NextTry > now) return false;	//单向屏蔽
			
			if(m_BanedTime > now) return false; //双向屏蔽

			if(connectable && m_Connective <=0) return false;

			if(m_LinkingRecord) return false; //已经有session正在连接，避免对一个地址同时发起两个连接

			if(m_LinkRecord & (1<<sess) ) return false; //本session已经有他的连接了

			if(m_OutLinkCount + m_InLinkCount >= SESSIONNUM ) return false; //对同一个地址建立了3个以上的连接，太多同地址连接影响对其他地址的连接
			
			if(m_iPort==0) return false; //不可连接的项目，也许是连接进来的

			return true;
		}

		void RecordLinking( unsigned int sess)
		{
			m_LinkingRecord |= (1<<sess);
		}

		void RecordLinkResult( unsigned int sess, bool ok)
		{
			m_LinkingRecord &= ~(1<<sess);

			if(ok)
			{//连接成功
				m_LinkRecord |= (1<<sess);

				if(m_Connective>=0) m_Connective++;
				else m_Connective=1;

				m_OutLinkCount++;

				assert(m_OutLinkCount>=1);

				//设置下一次连接间隔
				if(m_OutLinkCount + m_InLinkCount >= 2)
				{
					m_NextTry=GetTickCount()+ 60*1000+ (rand()%10)*1000;
				}
				else if(m_OutLinkCount + m_InLinkCount >= 1)
				{
					m_NextTry=GetTickCount()+ 10*1000+ (rand()%10)*1000;
				}
			}
			else
			{

				if(m_Connective>0) m_Connective=-1;
				else m_Connective--;

				m_nTimeOut=20; //以后采用20秒超时方案

				//是否已经建立了被动连接？
				if(m_LinkByCount > 0 && m_Connective <=-2) //连接了两次，但都失败了，却被对方连到
				{//对方是内网
					m_NextTry=GetTickCount()+ 7200*1000; //分，
				}
				else
				{
					switch(m_Connective)
					{
					case -1:
						m_NextTry=GetTickCount()+ 240*1000 + (rand()%240)*1000;	//120秒，
						break;
					case -2:
						m_NextTry=GetTickCount()+ 480*1000+ (rand()%480)*1000;  //4分，
						break;
					case -3:
						m_NextTry=GetTickCount()+ 960*1000+ (rand()%960)*1000;  //8分，
						break;
					default:
						m_NextTry=GetTickCount()+ 3600*1000+ (rand()%3600)*1000; //30分，
						break;
					}
				}
 
			}
		}

/*

新连接优先级别2008/01/28：

未主动连接的且没有被连接发生的，20	这是初始的状态
未主动连接的但已经被连接过的，当前连接数==0：15，争取反过去连接对方，虽然可能对方是连不到的
未主动连接的但已经被连接过的，当前连接数==1：10，争取反过去连接对方，虽然可能对方是连不到的
未主动连接的但已经被连接过的，当前连接数>=2: 3， 已经有两个连接了，不着急建立新连接

已经主动连接过的，曾经失败一次：7，
已经主动连接过的，曾经失败二次：4，如果有被对方连接则直接降低为0
已经主动连接过的，曾经失败三次以上：0，

已经主动连接过的，确认了可连接，当前发起的连接数为0的，12-n ，n为成功连接的次数
已经主动连接过的，确认了可连接，当前发起的连接数为1的，9-n , n为成功连接的次数
已经主动连接过的，确认了可连接，当前发起的连接数为>=2的，1
*/
		//连接优先级别，用于排序，新的数据应该优先，连接失败多的应该靠后
		int LinkPriority(unsigned int sessid)
		{

			if(m_Connective==0) 
			{//可连接性未确定的
				//m_LinkByCount是被对方连接的记数，只增
				if(m_LinkByCount==0)	return 20;
				else if(m_InLinkCount==0) return 15;
				else if(m_InLinkCount==1) return 10;
				else return 2;	//对方已经做了两个以上的连接给我们
			}
			else if(m_Connective >0) 
			{//已确定可连接的

				if(m_OutLinkCount==0)
				{
					if(m_ByteGot!=0 || m_ByteSend!=0) 
					{
						if(m_ByteGot==0)
						{
							return 1; //我们什么也没得到，不是个好连接，即使是无连接，也尽量避免连接这个
						}
						else if(m_ByteSend==0)
						{
							//我们没发送数据，可能是对方为种子
							return 18; //高优先连接

						}
						else
						{
							//有交换，两个都不等于0，交换比例越好则越连接他
							float gotsend=float(m_ByteGot)/float(m_ByteSend);
							if(gotsend>3.0f) return 17;
							else if(gotsend>2.0f) return 15;
							else if(gotsend>1.5f) return 13;
							else if(gotsend>1.2f) return 11;
							else if(gotsend>1.0f) return 9;
							else if(gotsend>0.95f) return 7;
							else if(gotsend>0.9f) return 6;
							else return 5;
						}
					}
					else
					{//从来没有发生过交换数据
						return MAX(1, 12-m_Connective); //m_Connective越大则优先越小，最低为1
					}
				}
				else if(m_OutLinkCount==1)
				{
					return MAX(1, 9-m_Connective); //m_Connective越大则优先越小，最低为1
				}
				else
				{
					return 1; //发起的连接已经达到了2或更多，不那么着急了。
				}

			}
			else //m_Connective < 0
			{//连接至少失败了一次的

				if(m_Connective==-1) return 7;
				else if(m_Connective==-2)
				{
					if(m_LinkByCount>0) return 1; //已经失败了两次，且被对方连接，估计对方是连不上了。
					else return 4;
				}
				else
				{ //失败次数太多了
					return 1;
				}

			}

		}

/*
typedef enum
{
	CR_LINKFAIL=0,		//连接失败
	CR_NOSHAKE=1,			//刚连接上就立即被对方正常关闭，这种情况可能是对方连接饱和或拒绝连接或没这个任务了
	CR_SHAKE_TIMEOUT=2,	//连接成功，也未被对方立即关闭，但没收到握手
	CR_SHAKE=3,			//发送了握手但对方正常关闭连接
	CR_SHAKE_NETERR=4,	//发送了握手后，网络错误连接中断后关闭，这种情况很可疑，应优先采用加密连接
	CR_BAD_SHAKE=5,		//我们得到了错误的握手信号，主动关闭连接
	CR_BIT_TIMEOUT=6,		//接收bitset超时
	CR_NOT_ACTIVE=7,		//对方没有激活包反馈过来，我们主动关闭连接
	CR_DATA_TIMEOUT=8,	//我们在等待数据中超时
	CR_BAD_DATA=9,		//数据校验错误
	CR_NETERR=10,			//除握手时其他情况下的网络错误导致的关闭连接
	CR_PEERCLOSE=11,		//对方主动正常地关闭连接
	CR_MYCLOSE=12,			//我们主动关闭连接，我们已完成，对方也完成，不需要连接它
	CR_PROTOCOL=13,		//协议错误关闭连接
	CR_SELF=14,			//连到了自己
	CR_NOTINTEREST=15,	//不感兴趣关闭
	CR_PICKCLOSE=16,	//选择性关闭, 说明对方下载优先级在我们这不够高
	CR_HONEST=17,
	CR_NOROOM=18,
} TCloseReason;
*/
		void RecordClose( unsigned int sess, TCloseReason res, bool accepted, std::string& peerid, CBTPiece* bitset)
		{
			unsigned int now=GetTickCount();

			m_LastClose=now;


			if(accepted)
			{
				m_InLinkCount--;

				assert(m_InLinkCount>=0);

				//接受的连接并不记录在m_LinkRecord内
				//m_BanedTime=now+60*1000;	//被剔除的进入的连接肯定不受欢迎，关他1分钟避免他老连
				//根据关闭的理由设置下次连接时间限制
				switch(res)
				{
				case CR_BIT_TIMEOUT:
					m_BanedTime=now+60*1000; 
					break;
				case CR_SHAKE_TIMEOUT:
					m_BanedTime=now+60*1000; //4min或许太长了
					break;
				case CR_SHAKE:
					m_NextTry=now+60*1000;
					break;
				case CR_NOSHAKE:
				case CR_SHAKE_NETERR:
				case CR_NOT_ACTIVE:
				case CR_DATA_TIMEOUT:
					m_NextTry=now+60*1000;
					break;
				case CR_BAD_SHAKE:
					m_BanedTime=now+600*1000;
					break;
				case CR_BAD_DATA:
					m_BanedTime=now+600*1000;
					break;
				case CR_NETERR:
					m_NextTry=now+15*1000;
					break;
				case CR_PEERCLOSE:
					{
						m_NextTry=now+15*1000;
					}
					break;
				case CR_MYCLOSE:
					{
						m_NextTry=now+90*1000;
					}
					break;
				case CR_PROTOCOL:
					m_BanedTime=now+300*1000;
					break;
				case CR_SELF:
					m_BanedTime=now+6000*1000;
					break;
				case CR_NOTINTEREST:
					m_BanedTime=now+60*1000;
					break;
				case CR_PICKCLOSE:
					{
						m_BanedTime=now+60*1000;
					}
					break;
				case CR_HONEST:
					m_BanedTime=now+900*1000; //关15分钟
					break;
				case CR_NOROOM:	//只有发起的连接才有这个
					m_NextTry=now+30*1000;
					break;
				case CR_NOENCRYPT:
					m_NextTry=now+10*1000; 	//如果选择是总用加密连接，这里可能就有问题了，可能反复连接
					break;
				default:
					m_NextTry=now+10*1000;
					break;
				}

			}
			else
			{
				m_OutLinkCount--;

				assert(m_OutLinkCount>=0);

				m_LinkRecord &= ~(1<<sess);

				//根据关闭的理由设置下次连接时间限制
				switch(res)
				{
				case CR_NOSHAKE:
					m_NextTry= now+60*1000; //对方在我们发送shake前关闭连接，显然，无法接受我们的连接
					break;
				case CR_SHAKE_TIMEOUT:
					m_NextTry= now+60*1000;
					break;
				case CR_SHAKE:
					m_NextTry=now+30*1000;
					break;
				case CR_SHAKE_NETERR:
					m_NextTry=now+15*1000;
					break;
				case CR_BAD_SHAKE:
					m_BanedTime=now+100*1000;
					break;
				case CR_BIT_TIMEOUT:
					m_NextTry=now+180*1000;
					break;
				case CR_NOT_ACTIVE:
					m_NextTry=now+15*1000;
					break;
				case CR_DATA_TIMEOUT:
					m_NextTry=now+15*1000;
					break;
				case CR_BAD_DATA:
					m_BanedTime= now+600*1000;
					break;
				case CR_NETERR:
					m_NextTry=15*1000;
					break;
				case CR_PEERCLOSE:
					m_NextTry=now+20*1000;
					break;
				case CR_MYCLOSE:
					m_NextTry=now+90*1000;
					break;
				case CR_PROTOCOL:
					m_NextTry=now+300+1000;
					break;
				case CR_SELF:
					m_BanedTime=now+1200*1000;
					break;
				case CR_NOTINTEREST:
					m_BanedTime=now+120*1000;
					break;
				case CR_PICKCLOSE:
					m_BanedTime=now+60*1000;
					break;
				case CR_HONEST:
					m_BanedTime=now+1200*1000; //关20分钟
					break;
				case CR_NOROOM:
					m_NextTry=now+30*1000; //+ (rand()%8000);
					break;
				case CR_NOENCRYPT:	//对方不接受加密连接
					m_NextTry=now+12*1000;	//如果选择是总用加密连接，这里可能就有问题了，可能反复连接
					break;
				default:
					m_NextTry=now+10*1000;
					break;
				}
			}

			if(m_EncryptRef<=0) m_EncryptRef--; //不确定对方的加密支持，变化一次

		//	m_ByteGot+=dsum;
		//	m_ByteSend+=usum;

			if(peerid.size()==20) m_PeerId=peerid; //有效的ID，记录

			if(bitset!=NULL)
			{
				if(m_pBitSet==NULL)
				{
					m_pBitSet=new CBTPiece(*bitset);
				}
				else
				{
					(*m_pBitSet)=(*bitset);
				}
			}


		}

		bool TryAccept(unsigned int sess)
		{

			m_LinkByCount++;	//这个是不减的，是计数，计算他总共连了多少次

			if( m_InLinkCount > 2 ) return false; //已经至少有三个进入的连接了，同一个地址不接受三个以上的进入连接
			if( m_OutLinkCount + m_InLinkCount > 2) return false; //已经至少有3个连接了，同一个地址双向连接不大于3个
			if( m_BanedTime > GetTickCount()) return false;

			m_InLinkCount++;	//这个是被动连接数
			assert(m_InLinkCount>0);

			if( m_Connective <=-2)
			{
				m_NextTry= GetTickCount() + 3600*1000;
			}

			return true;
		}

		void GiveUpAcceptPeerLink(unsigned int sess)
		{
			m_InLinkCount--;
		}

		//下载结束时调用，取消以前做的标记
		void ResetWhenFinish()
		{
			m_NextTry=0;
			m_BanedTime=0;
		}

		//因资源限制放弃连接尝试，没有结果
		void GiveUpLink(unsigned int sess)
		{
			m_LinkingRecord &= ~(1<<sess);
		}

		int CheckBitSet(unsigned int sess , std::string& peerid, CBTPiece& bitset)
		{
			//record bitset
			if(m_PeerId.empty()||m_PeerId.size()!=20)
			{
				m_PeerId=peerid;
			
			}


			if(m_pBitSet==NULL)
			{//first time got bitset
				m_pBitSet=new CBTPiece(bitset);
				return 0;
			}
			else
			{//多次获得bitset,新的bitset一定要>=老的

				int oldnum=m_pBitSet->GetSetedCount();
				int newnum=bitset.GetSetedCount();

				(*m_pBitSet)=bitset;
				return newnum - oldnum;

			}

			return 0;
		}

		void RecordLinkOkNoRoomClose(unsigned int sess)
		{
			m_LinkingRecord &= ~(1<<sess);

			if(m_Connective>=0) m_Connective++;
			else m_Connective=1;

			m_NextTry= MAX(m_NextTry, GetTickCount()+(25*m_Connective)*1000+(rand()%15)*1000);
			if(m_NextTry > 600*1000) m_NextTry=600*1000;
		}

		void RecordLinkOkPeerClose(unsigned int sess)
		{
			m_LinkingRecord &= ~(1<<sess);

			if(m_Connective>=0) m_Connective++;
			else m_Connective=1;

			if(m_LinkRecord)
			{
				m_NextTry= MAX(m_NextTry, GetTickCount()+25*1000+ (rand()%15)*1000);
			}
			else
			{
				m_NextTry= MAX(m_NextTry, GetTickCount()+(25*m_Connective)*1000+(rand()%15)*1000);

				if(m_NextTry > 600*1000) m_NextTry=600*1000;
			}
		}

		void PeerSupportEncryption(bool enc)
		{
			if(enc) m_EncryptRef=2;	//双数为支持加密
			else m_EncryptRef=1;	//单数为不支持加密
		}

		void GotChunk(int chunks)
		{
			m_ByteGot+=chunks;
		}

		void SendChunk(int chunks)
		{
			m_ByteSend+=chunks;
		}

		void PeerHaveNewPiece(int index)
		{
			if(m_pBitSet)
			{
				m_pBitSet->Set(index,true);
			}
		}

		float GetPeerProgress()
		{
			if(m_pBitSet)
			{
				return m_pBitSet->GetPersent();
			}
			else
			{
				return 0.0f;
			}
		}

		int GetCredit()
		{
			if(m_ByteGot==0 && m_ByteSend==0)
			{//初始态
				return 1;
			}
			else if(m_ByteGot==0)
			{
				return 0;
			}
			else
			{
				int base = m_ByteGot - m_ByteSend;
			
				return MAX(0, m_ByteGot/10 + base);
			}

		}

		bool CheckAccept()
		{
			unsigned int now=GetTickCount();

			if( m_BanedTime > now || m_OutLinkCount+m_InLinkCount > 2) 
			{
				m_LinkByCount++;	//这个是不减的，是计数，计算他总共连了多少次
				return false;		//拒绝连接时，记录被对方连接的次数
			}

			return true;	//可接受连接时，不在这里记录被连接次数
		}

		void CalSpeedTick(int span) //ms unit
		{
			unsigned int ds=m_ByteGot-m_LastByteGot;
			unsigned int us=m_ByteSend-m_LastByteSend;
			
			m_DownSpd=ds*16*1024*1000/span;
			m_UpSpd=us*16*1024*1000/span;

			m_LastByteGot=m_ByteGot;
			m_LastByteSend=m_ByteSend;
		}

		unsigned int	m_iIP;			//对方地址，网络序
		unsigned short	m_iPort;		//对方端口，网络序
		int				m_Connective;	//可连接性
		int				m_EncryptRef;	//是否使用加密连接的参考值, 0不确定，1确定支持，2确定不支持，<=0不确定，单数为尝试普通连接，双数尝试加密连接

		unsigned int	m_LastClose;	//最后的连接关闭时间
		unsigned int	m_NextTry;		//下一次尝试连接的时间

		unsigned int	m_DataAmount;	//上次发现的他的数据量，防欺骗
		unsigned int	m_OutLinkCount;	//当前我们发起建立的连接总数
		unsigned int	m_InLinkCount;	//对方发起的连接总数
		unsigned int	m_LinkingRecord;//我们哪个session正在发起连接? 避免相同session重复连接同一个地址
		unsigned int	m_LinkRecord;	//我们哪个session对他建立了连接? 避免相同session重复连接同一个地址
		unsigned int	m_BanedTime;	//禁止发起连接或接进连接的解除时间，0表示没有禁止
		unsigned int	m_LinkByCount;	//我们被这个地址连接的次数，累计计算，如果我们连不到这个地址，而这个地址能连到我们!=0，则这个连接相对重要
		unsigned int	m_ByteGot;		//从那里得到的数据总数，以16K为单位
		unsigned int	m_ByteSend;		//我们发送给他的数据总数，以16K为单位
		unsigned int	m_LastByteGot;		//从那里得到的数据总数，以16K为单位，上次计算值
		unsigned int	m_LastByteSend;		//我们发送给他的数据总数，以16K为单位，上次计算值
		unsigned int	m_nTimeOut;		//动态超时设置
		unsigned int	m_nShowTick;	//上次显示时间，避免快速刷新，最多一秒一次
		unsigned int	m_nStatus;		//状态位，choke, interest ...
		unsigned int	m_DownSpd;		//下行速度
		unsigned int	m_UpSpd;		//上行速度
		std::string		m_PeerId;		//获得的对方ID，可以统计从哪种ID上可以获得更多数据
		CBTPiece*		m_pBitSet;
	};

	typedef std::map<unsigned int, TPeerDetail> TPeerInfoMap; //前者是IP地址的变换hash(ip)


public:
	bool CheckAccept(unsigned int iip);
	int GetPeerCredit(unsigned int iip);
	void PeerHaveNewPieceNotice(unsigned int iip, int index);

	//void SetEventBack(int taskid, BTTASKEVENT eventback);
	//当发送一个片时，调用这个用于统计
	void SendChunk(unsigned int iip, int chunks);
	//当得到一个片时，调用这个用于统计
	void GotChunk(unsigned int iip, int chunks);

	void PeerSupportEncryption(unsigned int iip, bool enc);
	void LinkOkButPeerClose(unsigned int sessid, unsigned int iip);
	void GiveUpAcceptPeerLink(unsigned int sessid, unsigned int iip);
	void LinkOkButNoRoomClose(unsigned int sessid, unsigned int iip);
	unsigned int HashIP(unsigned int iip);
	bool AnyUnCheckedNode();

	int CheckBitSet(unsigned int sessid, std::string& peerid, unsigned int iip, CBTPiece& bitset);
	void GiveUpLink(unsigned int sessid, unsigned int iip);
	void AddAvialableBenliudPeerInfo(unsigned int iip, unsigned short iport);
	void DownloadFinish(bool finish=true);
	unsigned int GetConnecting();
	unsigned int GetPeerInitConnected();
	unsigned int GetMyInitConnected();
	unsigned int GetTotalConnected();
	unsigned int GetTotalPeer();
	void SetConnectionLimit(unsigned int limit);
	void SetConnectingLimit(unsigned int limit);
	bool TryAcceptPeerLink(unsigned int sessid,  unsigned int iip);
	void CloseReport(unsigned int sessid, unsigned int iip, TCloseReason reason, bool accepted, std::string &peerid, CBTPiece* bitset);
	void LinkReport(unsigned int sessid, unsigned int iip, bool ok);
	bool GetPeerInfoToLink(unsigned int sessid, bool connectable, unsigned int& iip, unsigned short& iport, int& encref, unsigned int& timeout);
	bool AddAvialablePeerInfo(unsigned int iip, unsigned short iport);
	CPeerCenter();
	virtual ~CPeerCenter();

protected:
	unsigned int	m_nConnection;	//已连接总数
	unsigned int	m_nMyInitConnection;	//我们发起形成的连接数
	unsigned int	m_nPeerInitConnection;	//对方发起形成的连接数

	unsigned int	m_nConnecting;	//正连接数

	unsigned int	m_nSessionConnecting[SESSIONNUM];
	unsigned int	m_nSessionConnection[SESSIONNUM];

	unsigned int	m_nTotalPeer;	//总数

	unsigned int	m_nConnectingLimit;
	unsigned int	m_nConnectionLimit;
	unsigned int	m_nLastCalSpeedTick;
	bool			m_UploadMode;
	
	SockLib::CMutex			m_Mutex;
	TPeerInfoMap	m_PeerInfoMap;		//老的，已经发生了连接的放这里

	int				m_nTaskId;

public:
	// the tick for calculate average speed
	void TickForCalSpeed(void);
};

#endif // !defined(AFX_PEERCENTER_H__BA66BF78_8038_43BF_B615_916A0B7F591B__INCLUDED_)

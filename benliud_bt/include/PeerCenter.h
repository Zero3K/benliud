/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


// PeerCenter.h: interface for the CPeerCenter class.
//
//////////////////////////////////////////////////////////////////////

//peer���ݹ�������
//�Ժ�peeradmin��ͳһ������ȡ����

#ifndef _PEERCENTER_H
#define _PEERCENTER_H

#include <string>
#include <map>
#include <vector>

#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif

#include <Tools.h>
#include <Mutex.h>
#include "datatype_def.h"
#include "BTPeer.h"
#include "sessiondef.h"
#include "BTPiece.h"

// Ensure MAX is defined as function-like macro after all includes
#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif
#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))

#ifdef _CHECK
extern void syslog(std::string info);
#endif


/*

���������ȼ���2008/01/28��

δ�������ӵ���û�б����ӷ����ģ�20	���ǳ�ʼ��״̬
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������==0��15����ȡ����ȥ���ӶԷ�����Ȼ���ܶԷ�����������
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������==1��10����ȡ����ȥ���ӶԷ�����Ȼ���ܶԷ�����������
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������>=2: 5�� �Ѿ������������ˣ����ż�����������

�Ѿ��������ӹ��ģ�����ʧ��һ�Σ�7��
�Ѿ��������ӹ��ģ�����ʧ�ܶ��Σ�4������б��Է�������ֱ�ӽ���Ϊ0
�Ѿ��������ӹ��ģ�����ʧ���������ϣ�0��

�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ0�ģ�12-n ��nΪ�ɹ����ӵĴ���
�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ1�ģ�9-n , nΪ�ɹ����ӵĴ���
�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ>=2�ģ�1
*/

/*
�Ѿ��������ӹ��ģ����û�гɹ�������ʧ��Խ������Խ�ͣ�ʧ��һ�Σ�3��ʧ�����Σ�2��ʧ����������1��
ʧ��1�������ҷ����������ӣ�������Ϊ0��

����ɹ����ӹ�������ǰ�����ӵģ�����Ϊ6�������ǰ��һ�����ӵģ�����Ϊ5�������ǰ�������������ӵģ�����Ϊ4




�������ȼ�����, ���ӷ�������ȼ����ο���������

û�з������ӵģ���ߣ����ȼ�10

���ӳɹ��������ҵ�ǰ������<1�� ���ȼ�7

���ӳɹ��������ҵ�ǰ������<2�� ���ȼ�6


����ʧ��һ�εģ���ֹ����ʱ��=1*30 �����ȼ�3

����ʧ�����εģ���ֹ����ʱ��=2*30�� ���ȼ�2

����ʧ��n�εģ���ֹ����ʱ��=2^n*30�����ȼ�1


����ʧ��һ�����ϣ����ҷ����˱������ӵģ���ֹ����ʱ��=3600s �����ȼ�0

ÿ�η������Ӻ����ټ��15���Ժ��ٳ������ӡ����ڵ�һ�����ӳɹ��ģ���һ��
����������5������������Լӿ������ٶȡ�

*/


class CBTPiece;


class CPeerCenter  
{

	struct _t
	{
		unsigned int	ip;
		unsigned int	link;	//��������
		float			prog;	//4�ֽڣ�ϣ��64λ��Ҳ��4�ֽ�
		int				encref;	//4�ֽ�
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
			m_nTimeOut=20;	//��̬�ģ�ÿ������ʧ��+3ֱ��60��

			m_pBitSet=NULL;
			m_nShowTick=0;

		}

		virtual ~TPeerDetail()
		{
			if(m_pBitSet!=NULL)
				delete m_pBitSet;
		}

		//�Ƿ�δ���Ľڵ�
		bool UnCheckedNode()
		{
			return m_Connective==0;
		}

		//�Ƿ�����ڷ������ӣ�
		bool AvialableForLink(unsigned int sess, bool connectable, unsigned int now) const
		{
			if(m_NextTry > now) return false;	//��������
			
			if(m_BanedTime > now) return false; //˫������

			if(connectable && m_Connective <=0) return false;

			if(m_LinkingRecord) return false; //�Ѿ���session�������ӣ������һ����ַͬʱ������������

			if(m_LinkRecord & (1<<sess) ) return false; //��session�Ѿ�������������

			if(m_OutLinkCount + m_InLinkCount >= SESSIONNUM ) return false; //��ͬһ����ַ������3�����ϵ����ӣ�̫��ͬ��ַ����Ӱ���������ַ������
			
			if(m_iPort==0) return false; //�������ӵ���Ŀ��Ҳ�������ӽ�����

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
			{//���ӳɹ�
				m_LinkRecord |= (1<<sess);

				if(m_Connective>=0) m_Connective++;
				else m_Connective=1;

				m_OutLinkCount++;

				assert(m_OutLinkCount>=1);

				//������һ�����Ӽ��
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

				m_nTimeOut=20; //�Ժ����20�볬ʱ����

				//�Ƿ��Ѿ������˱������ӣ�
				if(m_LinkByCount > 0 && m_Connective <=-2) //���������Σ�����ʧ���ˣ�ȴ���Է�����
				{//�Է�������
					m_NextTry=GetTickCount()+ 7200*1000; //�֣�
				}
				else
				{
					switch(m_Connective)
					{
					case -1:
						m_NextTry=GetTickCount()+ 240*1000 + (rand()%240)*1000;	//120�룬
						break;
					case -2:
						m_NextTry=GetTickCount()+ 480*1000+ (rand()%480)*1000;  //4�֣�
						break;
					case -3:
						m_NextTry=GetTickCount()+ 960*1000+ (rand()%960)*1000;  //8�֣�
						break;
					default:
						m_NextTry=GetTickCount()+ 3600*1000+ (rand()%3600)*1000; //30�֣�
						break;
					}
				}
 
			}
		}

/*

���������ȼ���2008/01/28��

δ�������ӵ���û�б����ӷ����ģ�20	���ǳ�ʼ��״̬
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������==0��15����ȡ����ȥ���ӶԷ�����Ȼ���ܶԷ�����������
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������==1��10����ȡ����ȥ���ӶԷ�����Ȼ���ܶԷ�����������
δ�������ӵĵ��Ѿ������ӹ��ģ���ǰ������>=2: 3�� �Ѿ������������ˣ����ż�����������

�Ѿ��������ӹ��ģ�����ʧ��һ�Σ�7��
�Ѿ��������ӹ��ģ�����ʧ�ܶ��Σ�4������б��Է�������ֱ�ӽ���Ϊ0
�Ѿ��������ӹ��ģ�����ʧ���������ϣ�0��

�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ0�ģ�12-n ��nΪ�ɹ����ӵĴ���
�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ1�ģ�9-n , nΪ�ɹ����ӵĴ���
�Ѿ��������ӹ��ģ�ȷ���˿����ӣ���ǰ�����������Ϊ>=2�ģ�1
*/
		//�������ȼ������������µ�����Ӧ�����ȣ�����ʧ�ܶ��Ӧ�ÿ���
		int LinkPriority(unsigned int sessid)
		{

			if(m_Connective==0) 
			{//��������δȷ����
				//m_LinkByCount�Ǳ��Է����ӵļ�����ֻ��
				if(m_LinkByCount==0)	return 20;
				else if(m_InLinkCount==0) return 15;
				else if(m_InLinkCount==1) return 10;
				else return 2;	//�Է��Ѿ������������ϵ����Ӹ�����
			}
			else if(m_Connective >0) 
			{//��ȷ�������ӵ�

				if(m_OutLinkCount==0)
				{
					if(m_ByteGot!=0 || m_ByteSend!=0) 
					{
						if(m_ByteGot==0)
						{
							return 1; //����ʲôҲû�õ������Ǹ������ӣ���ʹ�������ӣ�Ҳ���������������
						}
						else if(m_ByteSend==0)
						{
							//����û�������ݣ������ǶԷ�Ϊ����
							return 18; //����������

						}
						else
						{
							//�н�����������������0����������Խ����Խ������
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
					{//����û�з�������������
						return MAX(1, 12-m_Connective); //m_ConnectiveԽ��������ԽС�����Ϊ1
					}
				}
				else if(m_OutLinkCount==1)
				{
					return MAX(1, 9-m_Connective); //m_ConnectiveԽ��������ԽС�����Ϊ1
				}
				else
				{
					return 1; //����������Ѿ��ﵽ��2����࣬����ô�ż��ˡ�
				}

			}
			else //m_Connective < 0
			{//��������ʧ����һ�ε�

				if(m_Connective==-1) return 7;
				else if(m_Connective==-2)
				{
					if(m_LinkByCount>0) return 1; //�Ѿ�ʧ�������Σ��ұ��Է����ӣ����ƶԷ����������ˡ�
					else return 4;
				}
				else
				{ //ʧ�ܴ���̫����
					return 1;
				}

			}

		}

/*
typedef enum
{
	CR_LINKFAIL=0,		//����ʧ��
	CR_NOSHAKE=1,			//�������Ͼ��������Է������رգ�������������ǶԷ����ӱ��ͻ�ܾ����ӻ�û���������
	CR_SHAKE_TIMEOUT=2,	//���ӳɹ���Ҳδ���Է������رգ���û�յ�����
	CR_SHAKE=3,			//���������ֵ��Է������ر�����
	CR_SHAKE_NETERR=4,	//���������ֺ�������������жϺ�رգ���������ܿ��ɣ�Ӧ���Ȳ��ü�������
	CR_BAD_SHAKE=5,		//���ǵõ��˴���������źţ������ر�����
	CR_BIT_TIMEOUT=6,		//����bitset��ʱ
	CR_NOT_ACTIVE=7,		//�Է�û�м�����������������������ر�����
	CR_DATA_TIMEOUT=8,	//�����ڵȴ������г�ʱ
	CR_BAD_DATA=9,		//����У�����
	CR_NETERR=10,			//������ʱ��������µ���������µĹر�����
	CR_PEERCLOSE=11,		//�Է����������عر�����
	CR_MYCLOSE=12,			//���������ر����ӣ���������ɣ��Է�Ҳ��ɣ�����Ҫ������
	CR_PROTOCOL=13,		//Э�����ر�����
	CR_SELF=14,			//�������Լ�
	CR_NOTINTEREST=15,	//������Ȥ�ر�
	CR_PICKCLOSE=16,	//ѡ���Թر�, ˵���Է��������ȼ��������ⲻ����
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

				//���ܵ����Ӳ�����¼��m_LinkRecord��
				//m_BanedTime=now+60*1000;	//���޳��Ľ�������ӿ϶����ܻ�ӭ������1���ӱ���������
				//���ݹرյ����������´�����ʱ������
				switch(res)
				{
				case CR_BIT_TIMEOUT:
					m_BanedTime=now+60*1000; 
					break;
				case CR_SHAKE_TIMEOUT:
					m_BanedTime=now+60*1000; //4min����̫����
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
					m_BanedTime=now+900*1000; //��15����
					break;
				case CR_NOROOM:	//ֻ�з�������Ӳ������
					m_NextTry=now+30*1000;
					break;
				case CR_NOENCRYPT:
					m_NextTry=now+10*1000; 	//���ѡ�������ü������ӣ�������ܾ��������ˣ����ܷ�������
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

				//���ݹرյ����������´�����ʱ������
				switch(res)
				{
				case CR_NOSHAKE:
					m_NextTry= now+60*1000; //�Է������Ƿ���shakeǰ�ر����ӣ���Ȼ���޷��������ǵ�����
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
					m_BanedTime=now+1200*1000; //��20����
					break;
				case CR_NOROOM:
					m_NextTry=now+30*1000; //+ (rand()%8000);
					break;
				case CR_NOENCRYPT:	//�Է������ܼ�������
					m_NextTry=now+12*1000;	//���ѡ�������ü������ӣ�������ܾ��������ˣ����ܷ�������
					break;
				default:
					m_NextTry=now+10*1000;
					break;
				}
			}

			if(m_EncryptRef<=0) m_EncryptRef--; //��ȷ���Է��ļ���֧�֣��仯һ��

		//	m_ByteGot+=dsum;
		//	m_ByteSend+=usum;

			if(peerid.size()==20) m_PeerId=peerid; //��Ч��ID����¼

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

			m_LinkByCount++;	//����ǲ����ģ��Ǽ������������ܹ����˶��ٴ�

			if( m_InLinkCount > 2 ) return false; //�Ѿ���������������������ˣ�ͬһ����ַ�������������ϵĽ�������
			if( m_OutLinkCount + m_InLinkCount > 2) return false; //�Ѿ�������3�������ˣ�ͬһ����ַ˫�����Ӳ�����3��
			if( m_BanedTime > GetTickCount()) return false;

			m_InLinkCount++;	//����Ǳ���������
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

		//���ؽ���ʱ���ã�ȡ����ǰ���ı��
		void ResetWhenFinish()
		{
			m_NextTry=0;
			m_BanedTime=0;
		}

		//����Դ���Ʒ������ӳ��ԣ�û�н��
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
			{//��λ��bitset,�µ�bitsetһ��Ҫ>=�ϵ�

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
			if(enc) m_EncryptRef=2;	//˫��Ϊ֧�ּ���
			else m_EncryptRef=1;	//����Ϊ��֧�ּ���
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
			{//��ʼ̬
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
				m_LinkByCount++;	//����ǲ����ģ��Ǽ������������ܹ����˶��ٴ�
				return false;		//�ܾ�����ʱ����¼���Է����ӵĴ���
			}

			return true;	//�ɽ�������ʱ�����������¼�����Ӵ���
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

		unsigned int	m_iIP;			//�Է���ַ��������
		unsigned short	m_iPort;		//�Է��˿ڣ�������
		int				m_Connective;	//��������
		int				m_EncryptRef;	//�Ƿ�ʹ�ü������ӵĲο�ֵ, 0��ȷ����1ȷ��֧�֣�2ȷ����֧�֣�<=0��ȷ��������Ϊ������ͨ���ӣ�˫�����Լ�������

		unsigned int	m_LastClose;	//�������ӹر�ʱ��
		unsigned int	m_NextTry;		//��һ�γ������ӵ�ʱ��

		unsigned int	m_DataAmount;	//�ϴη��ֵ�����������������ƭ
		unsigned int	m_OutLinkCount;	//��ǰ���Ƿ���������������
		unsigned int	m_InLinkCount;	//�Է��������������
		unsigned int	m_LinkingRecord;//�����ĸ�session���ڷ�������? ������ͬsession�ظ�����ͬһ����ַ
		unsigned int	m_LinkRecord;	//�����ĸ�session��������������? ������ͬsession�ظ�����ͬһ����ַ
		unsigned int	m_BanedTime;	//��ֹ�������ӻ�ӽ����ӵĽ��ʱ�䣬0��ʾû�н�ֹ
		unsigned int	m_LinkByCount;	//���Ǳ������ַ���ӵĴ������ۼƼ��㣬������������������ַ���������ַ����������!=0����������������Ҫ
		unsigned int	m_ByteGot;		//������õ���������������16KΪ��λ
		unsigned int	m_ByteSend;		//���Ƿ��͸�����������������16KΪ��λ
		unsigned int	m_LastByteGot;		//������õ���������������16KΪ��λ���ϴμ���ֵ
		unsigned int	m_LastByteSend;		//���Ƿ��͸�����������������16KΪ��λ���ϴμ���ֵ
		unsigned int	m_nTimeOut;		//��̬��ʱ����
		unsigned int	m_nShowTick;	//�ϴ���ʾʱ�䣬�������ˢ�£����һ��һ��
		unsigned int	m_nStatus;		//״̬λ��choke, interest ...
		unsigned int	m_DownSpd;		//�����ٶ�
		unsigned int	m_UpSpd;		//�����ٶ�
		std::string		m_PeerId;		//��õĶԷ�ID������ͳ�ƴ�����ID�Ͽ��Ի�ø�������
		CBTPiece*		m_pBitSet;
	};

	typedef std::map<unsigned int, TPeerDetail> TPeerInfoMap; //ǰ����IP��ַ�ı任hash(ip)


public:
	bool CheckAccept(unsigned int iip);
	int GetPeerCredit(unsigned int iip);
	void PeerHaveNewPieceNotice(unsigned int iip, int index);

	//void SetEventBack(int taskid, BTTASKEVENT eventback);
	//������һ��Ƭʱ�������������ͳ��
	void SendChunk(unsigned int iip, int chunks);
	//���õ�һ��Ƭʱ�������������ͳ��
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
	unsigned int	m_nConnection;	//����������
	unsigned int	m_nMyInitConnection;	//���Ƿ����γɵ�������
	unsigned int	m_nPeerInitConnection;	//�Է������γɵ�������

	unsigned int	m_nConnecting;	//��������

	unsigned int	m_nSessionConnecting[SESSIONNUM];
	unsigned int	m_nSessionConnection[SESSIONNUM];

	unsigned int	m_nTotalPeer;	//����

	unsigned int	m_nConnectingLimit;
	unsigned int	m_nConnectionLimit;
	unsigned int	m_nLastCalSpeedTick;
	bool			m_UploadMode;
	
	SockLib::CMutex			m_Mutex;
	TPeerInfoMap	m_PeerInfoMap;		//�ϵģ��Ѿ����������ӵķ�����

	int				m_nTaskId;

public:
	// the tick for calculate average speed
	void TickForCalSpeed(void);
};

#endif // !defined(AFX_PEERCENTER_H__BA66BF78_8038_43BF_B615_916A0B7F591B__INCLUDED_)

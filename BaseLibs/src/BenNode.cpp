// BenNode.cpp: implementation of the CBenNode class.
//
//////////////////////////////////////////////////////////////////////
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/BenNode.h"
#include <deque>
#include <assert.h>
#include <memory.h> //for memcpy under linux
#include <stdio.h> //for sprintf under linux
#include <stdlib.h> //for atoll and atoi
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace BencodeLib
{
	CBenNode::CBenNode(CBenNode* parent,_NODETYPE nt,CBenNode* keyval)
	{
		m_nNodeType=nt;		//root should beNull
		m_pParent=parent;	//root dictionary will have NULL parent.
		m_pOperator=NULL;	//the child we operate on.
		m_KeyValue=keyval;
		m_nValue=0;
		m_pChild.clear();

	}

	CBenNode::~CBenNode()
	{
		Clean();
	}


	bool CBenNode::OpenDictionary()
	{
		if(m_nNodeType==beNull)
		{//open a dict on empty node
			m_nNodeType=beDict;
			return true;

		}
		else
		{//node isn't empty
			if(m_nNodeType==beInt||m_nNodeType==beString)
			{//wrong node type
				return false;
			}

			if(m_nNodeType==beList)
			{//in a list node open a new dict
				if(m_pOperator==NULL)
				{
					//make a new child node to contain data
					CBenNode* nn=new CBenNode(this,beDict);
					//insert to child list.
					m_pChild.push_back(nn);
					//make current operator is new node
					m_pOperator=nn;	

					return true;
				}
				else
				{
					return m_pOperator->OpenDictionary();
				}
			}

			if(m_nNodeType==beDict)
			{//in a dict open a new dict

				if(m_pOperator==NULL)
				{
					//check the last element if waitting keyvalue
					if(!m_pChild.empty())
					{
						if(m_pChild.back()->GetKeyValue()==NULL)
						{
							//make a new child node to contain data
							CBenNode* nn=new CBenNode(this,beDict);
							//set the keyvalue as new list.
							m_pChild.back()->SetKeyValue(nn);
							//make current operator is new node
							m_pOperator=nn;	

							return true;

						}
					}

					return false; //create dict on a dict but not on a child keyvalue!

				}
				else
				{
					return m_pOperator->OpenDictionary();
				}
			}
			//wrong node type
			return false;

		}

	}

	bool CBenNode::CloseDictionary()
	{

		if(m_nNodeType==beInt||m_nNodeType==beString||m_nNodeType==beDictPair)
		{
			return false;
		}

		if(m_nNodeType==beList && m_pOperator==NULL)
		{
			return false;
		}

		if(m_nNodeType==beDict && m_pOperator==NULL)
		{
			return true;
		}

		//think about the list chain, only the bottom list should be closed!

		if((m_nNodeType==beDict || m_nNodeType==beList) && m_pOperator!=NULL)
		{
			if( m_pOperator->CloseDictionary() )
			{
				m_pOperator=NULL;
				//return false; //only one list should be closed
			}

			return false;
		}

		return false; //wrong node type;
	}

	bool CBenNode::OpenList()
	{

		if(m_nNodeType==beNull)
		{//open a list on empty node
			m_nNodeType=beList;
			return true;

		}
		else
		{//node isn't empty
			if(m_nNodeType==beInt||m_nNodeType==beString)
			{//wrong node type
				return false;
			}

			if(m_nNodeType==beDict)
			{//maybe dictpair is waitting a keyvalue,if yes,this open should on the keyvalue
				if(!m_pChild.empty())
				{

					if(m_pChild.back()->GetKeyValue()==NULL)
					{
						//make a new child node to contain data
						CBenNode* nn=new CBenNode(this,beList);
						//set the keyvalue as new list.
						m_pChild.back()->SetKeyValue(nn);

						//make current operator is new node
						m_pOperator=nn;	

						return true;

					}
				}
			}

			if(m_nNodeType==beList||m_nNodeType==beDict)
			{//in a list node open a new list
				if(m_pOperator==NULL)
				{
					//make a new child node to contain data
					CBenNode* nn=new CBenNode(this,beList);
					//insert to child list.
					m_pChild.push_back(nn);
					//make current operator is new node
					m_pOperator=nn;	

					return true;
				}
				else
				{
					return m_pOperator->OpenList();
				}
			}

			//wrong node type
			return false;

		}

	}

	bool CBenNode::CloseList()
	{
		if(m_nNodeType==beInt||m_nNodeType==beString)
		{
			return false;
		}

		if(m_nNodeType==beList && m_pOperator==NULL)
		{
			return true;
		}

		if(m_nNodeType==beDict && m_pOperator==NULL)
		{
			return false;
		}

		//think about the list chain, only the bottom list should be closed!

		if((m_nNodeType==beList || m_nNodeType==beDict) && m_pOperator!=NULL)
		{
			if( m_pOperator->CloseList() )
			{
				m_pOperator=NULL;
				return false; //only one list should be closed
			}

			return false;
		}

		return false; //wrong node type;

	}



	bool CBenNode::AddValue(llong value)
	{
		if(m_nNodeType==beNull||m_nNodeType==beInt)
		{
			m_nValue=value;
			m_nNodeType=beInt;
			return true;
		}
		else
		{
			if(m_nNodeType==beList && m_pOperator==NULL)
			{//make a new int node,and insert it to list
				CBenNode* nn=new CBenNode(this,beInt);
				nn->AddValue(value);
				m_pChild.push_back(nn);
				return true;
			}

			if(m_nNodeType==beList && m_pOperator!=NULL)
			{
				return m_pOperator->AddValue(value);
			}

			if(m_nNodeType==beDict && m_pOperator==NULL)
			{
				//try to locate the last element in list
				//and find if the key is empty, if empty key, this value is key else is value
				if(m_pChild.empty())
				{
					return false; //beInt can't be a key .
				}
				else
				{
					CBenNode* last=m_pChild.back();
					if(last->GetKeyValue()==NULL)
					{
						CBenNode *nk=new CBenNode(this,beInt);
						nk->AddValue(value);
						last->SetKeyValue(nk);
						return true;
					}
					else
					{
						return false;//beInt can't be a key .
					}
				}
			}//if(m_nNodeType==beDict && m_pOperator==NULL)

			if(m_nNodeType==beDict && m_pOperator!=NULL)
			{
				return m_pOperator->AddValue(value);

			}

			return false;//wrong node type;
		}

		return false;
	}

	bool CBenNode::AddValue(std::string value)
	{
		return AddValue(value.data(), value.size());
	}
	//the value maybe a binary string,so need len to indicate length
	bool CBenNode::AddValue(const char* value,int len)
	{
		if(m_nNodeType==beNull||m_nNodeType==beString)
		{
			m_sValue.append(value,len);
			m_nNodeType=beString;
			return true;
		}
		else
		{
			if(m_nNodeType==beList && m_pOperator==NULL)
			{//make a new int node,and insert it to list
				CBenNode* nn=new CBenNode(this,beString);
				nn->AddValue(value,len);
				m_pChild.push_back(nn);
				return true;
			}

			if(m_nNodeType==beList && m_pOperator!=NULL)
			{
				return m_pOperator->AddValue(value,len);
			}

			if(m_nNodeType==beDict && m_pOperator==NULL)
			{
				//try to locate the last element in list
				//and find if the key is empty, if empty key, this value is key else is value
				if(m_pChild.empty())
				{
					//the dict data node we treate it as beNull
					CBenNode* nn=new CBenNode(this,beDictPair);
					nn->SetKey(value,len); //the key value remain NULL,wait next input. a key should not be a binary string
					m_pChild.push_back(nn);
					return true;
				}
				else
				{//already have child
					CBenNode* last=m_pChild.back(); //get last child
					if(last->GetKeyValue()==NULL)
					{//last child no keyvalue
						CBenNode* nk=new CBenNode(this,beString);
						nk->AddValue(value,len);
						last->SetKeyValue(nk); //set the value as keyvalue
						return true;
					}
					else
					{//need create a new node, treate value as a new key
						CBenNode* nn=new CBenNode(this,beDictPair);
						nn->SetKey(value,len); //the key value remain NULL,a key should not be a binary string
						m_pChild.push_back(nn);	
						return true;
					}
				}

				return true;
			}//if(m_nNodeType==beDict && m_pOperator==NULL)

			if(m_nNodeType==beDict && m_pOperator!=NULL)
			{
				return m_pOperator->AddValue(value,len);
			}

			return false; //wrong node type;
		}
	}

	char* CBenNode::GetKey()
	{
		return (char*)m_sKey.c_str();
	}

	void CBenNode::SetKey(const char *value,int len)
	{
		char *buf=new char[len+1];
		memcpy(buf,value,len);
		buf[len]='\0';
		m_sKey=std::string(buf);
		delete[] buf;
	}

	CBenNode* CBenNode::GetKeyValue()
	{
		return m_KeyValue;
	}

	void CBenNode::SetKeyValue(CBenNode *kv)
	{
		m_KeyValue=kv;
	}

	CBenNode* CBenNode::FindKeyValue(char *key)
	{
		//try to find the key and return the value
		if(m_nNodeType==beDictPair) //maybe a dictionary left node
		{
			if(m_sKey==std::string(key))
			{
				return m_KeyValue;
			}
		}

		if(m_nNodeType==beInt||m_nNodeType==beString)
		{
			return NULL;
		}
		//if this is a dict

		if(m_nNodeType==beDict||m_nNodeType==beList)
		{
			CBenNode* pret=NULL;
			//try every node
			std::vector<CBenNode*>::const_iterator it;
			for(it=m_pChild.begin();it!=m_pChild.end();it++)
			{
				pret=(*it)->FindKeyValue(key);
				if(pret!=NULL) return pret;
			}
		}

		return NULL; //other node type,wrong
	}

	_NODETYPE CBenNode::GetType()
	{
		return m_nNodeType;
	}

	int CBenNode::GetNumberOfList()
	{
		if(m_nNodeType!=beList) return 0;

		return m_pChild.size();
	}

	//seq base on 0
	CBenNode* CBenNode::GetListMember(int seq)
	{
		if(seq<0||seq>=int(m_pChild.size())) return NULL;
		return m_pChild[seq];
	}

	llong CBenNode::GetIntValue()
	{
		return m_nValue;
	}

	void CBenNode::Clean()
	{
		if(m_nNodeType==beList||m_nNodeType==beDict)
		{
			//have child when beList or beDict
			std::vector<CBenNode*>::iterator it;
			for(it=m_pChild.begin();it!=m_pChild.end();it++)
			{
				CBenNode* p=*it;

				if(*it)	delete (*it);
			}

			m_pChild.clear();
		}

		if(m_KeyValue!=NULL) {
			delete m_KeyValue;
			m_KeyValue=NULL;
		}

		m_nNodeType=beNull;
	}

	//no recursly get value, just in this layer
	CBenNode* CBenNode::GetKeyValue(char *key)
	{
		if(m_nNodeType!=beDict) return NULL;

		//yes ,this is a dict, find the key in child list
		std::vector<CBenNode*>::const_iterator it;
		for(it=m_pChild.begin();it!=m_pChild.end();it++)
		{
			if(strcmp((*it)->GetKey(),key)==0 && (*it)->GetType()==beDictPair)
			{
				return (*it)->GetKeyValue();
			}
		}

		return NULL;
	}

	int CBenNode::GetNumberOfDict()
	{
		if(m_nNodeType!=beDict) return 0;

		return m_pChild.size();
	}

	void CBenNode::GetStringValue(std::string &str)
	{
		str=m_sValue;
	}

	void	CBenNode::EncodeToStream(CBenNode* pNode, std::string& stream)
	{
		if(pNode==NULL) return;

		if(pNode->GetType()==beList)
		{
			stream.append(1,'l');

			for(int i=0;i<pNode->GetNumberOfList();i++)
			{
				EncodeToStream(pNode->GetListMember(i),stream);
			}

			stream.append(1,'e');
		}
		else if(pNode->GetType()==beDict)
		{
			stream.append(1,'d');

			for(int i=0;i<pNode->GetNumberOfDict();i++)
			{
				EncodeToStream(pNode->GetListMember(i),stream);
			}

			stream.append(1,'e');
		}
		else if(pNode->GetType()==beInt)
		{
			//i34e
			char tbuf[28];
#ifdef WIN32
			sprintf_s(tbuf,28,"i%I64de",pNode->GetIntValue()); 
#else
			sprintf(tbuf,"i%llde",pNode->GetIntValue());
#endif
			stream.append(tbuf, strlen(tbuf));

		}
		else if(pNode->GetType()==beString)
		{
			//5:abcde
			std::string str;
			pNode->GetStringValue(str);

			char tbuf[25];
#ifdef WIN32
			sprintf_s(tbuf,25,"%d:",str.size());
#else
			sprintf(tbuf,"%d:",str.size());
#endif
			stream.append(tbuf, strlen(tbuf));

			stream.append(str);
		}
		else if(pNode->GetType()==beNull)
		{
			return;
		}
		else if(pNode->GetType()==beDictPair)
		{
			//output key
			char tbuf[25];
			std::string key=pNode->GetKey();

#ifdef WIN32
			sprintf_s(tbuf,25,"%d:",key.size());
#else
			sprintf(tbuf,"%d:",key.size());
#endif
			stream.append(tbuf, strlen(tbuf));

			stream.append(key);

			//output keyvalue
			EncodeToStream(pNode->GetKeyValue(),stream);
		}
		else
		{
			return ;
		}
	}

	void CBenNode::ToStream(std::string& stream)
	{
		stream.clear();
		EncodeToStream(this, stream);
	}

	int CBenNode::FromStream(std::string& stream)
	{
		Clean();

		std::deque<int> OpQueue;

		char buf[512];

		char* phead= (char*)stream.data();
		char* pmove= (char*)stream.data();
		int maxsize= stream.size();

		while(pmove-phead<maxsize)
		{
			switch(*pmove)
			{
			case 'd':
				OpenDictionary();
				OpQueue.push_back(0); //dict=0
				pmove++;
				break;
			case 'l':
				OpenList();
				OpQueue.push_back(1); //list=1
				pmove++;
				break;
			case 'i':
				{//find the 'e' for number end
					pmove++;
					char *begin=pmove;
					while(*pmove!='e' && pmove-phead<maxsize) pmove++;

					if(*pmove=='e') 
					{
						memcpy(buf,begin,pmove-begin);
						buf[pmove-begin]='\0';
#ifdef WIN32
						llong iv=_atoi64(buf);
#else
						llong iv=atoll(buf);
#endif
						AddValue(iv);
						pmove++;
					}
					else
					{
						return -4;
					}
				}
				break;
			case 'e':
				if(OpQueue.empty()) {
					return -5;
				}

				pmove++;

				if(OpQueue.back()==1)
				{
					OpQueue.pop_back();
					CloseList();
				}
				else 
				{
					OpQueue.pop_back();
					CloseDictionary();

					//because this CloseDictionary() may close the root dictionary,
					//and some torrent still have data after the main dictionary closed,
					//so , if the root dict is closed we end reading the data
					if(OpQueue.empty()) goto errquit;
				}

				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					//find the number
					char* begin=pmove;
					while(*pmove>='0' && *pmove<='9') pmove++;
					if(*pmove!=':') {
						return -6;
					}
					memcpy(buf,begin,pmove-begin);
					buf[pmove-begin]='\0';

					int slen=atoi(buf);

					pmove++;//skip ":"

					AddValue(pmove,slen);
					pmove+=slen;
				}
				break;
			default:
				//some torrent may extern the key that we don't recgnize
				//so if we don't know the key, we just skip and quit.
				//or it will put a text end char '0x0A' at the tail of file
				//printf("key=%02X don't know",*pmove);
				//printf("pmove-fbuf=%d, flen=%d\n", pmove-fbuf, flen);
				goto errquit;
			}
		}

errquit:
		//some torrent not close all opend list or dictionary at end.
		int left=OpQueue.size();
		if(left==0) return 0;
		else return -6;
	}

}
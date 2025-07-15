/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

// BenNode.h: interface for the CBenNode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BENNODE_H
#define _BENNODE_H

#pragma once


#include <string>
#include <vector>

#ifndef llong
#ifdef WIN32
typedef __int64 llong;
#else
typedef long long llong;
#endif
#endif

namespace BencodeLib
{
	//�ڵ������Ҷ��Ҳ�����Ǹ����Ǹ��ͱ�����LIST,��DICT��
	//��Ҷ�Ӿͱ������ַ���,�������ֵ�ģ�KEY��VALUE������ʱ������beDictPair
	enum _NODETYPE{ beNull, beString, beInt, beList, beDict, beDictPair };

	class CBenNode  
	{
	public:
		void GetStringValue(std::string &str);
		int GetNumberOfDict();
		CBenNode* GetKeyValue(char* key);
		void Clean();
		llong GetIntValue();
		CBenNode* GetListMember(int seq);
		int GetNumberOfList();
		_NODETYPE GetType();
		CBenNode* FindKeyValue(char* key);
		void SetKeyValue(CBenNode* kv);
		CBenNode* GetKeyValue();
		void SetKey(const char* value,int len);
		char* GetKey();
		bool CloseList();
		bool OpenList();
		bool CloseDictionary();
		bool OpenDictionary();
		bool AddValue(const char* value,int len);
		bool AddValue(llong value);
		bool AddValue(std::string value);
		void ToStream(std::string& stream);
		int FromStream(std::string& stream);

		CBenNode(CBenNode* parent=NULL,_NODETYPE nt=beNull,CBenNode* keyval=NULL);
		virtual ~CBenNode();

	private:
		void EncodeToStream(CBenNode* pNode, std::string& stream);

	protected:

		_NODETYPE	m_nNodeType;//if type=beNull, then this node have no data.
		llong		m_nValue;	//if type = beInt, here is the value
		CBenNode*	m_pParent;  //the upper layer of node
		CBenNode*	m_pOperator;//current operate node, it's a child node,if NULL,this is used for current node.
		CBenNode*   m_KeyValue; //if this is dictionary data node,here is the keydata.
		std::string m_sKey;		//the node key for dict type.
		std::string m_sValue; //if type = beString ,here is the value.

		std::vector<CBenNode*> m_pChild;	//if type = beList or beDict, this value point to m_nValue of pointer of CBenNode

	};

}
#endif

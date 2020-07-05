/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// DBItem.h: interface for the CDBItem class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DBITEM_H
#define _DBITEM_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif



#include <Tools.h>

class CDBItem  
{
public:
	CDBItem();
	virtual ~CDBItem();

	CDBItem(unsigned int iip, unsigned short iport);
	CDBItem(const unsigned char* ip_port);
	//CDBItem(const CDBItem & item);
		
	bool IsExpired(unsigned int  now) const;
	const unsigned char* GetData() const {return item;}
	
	CDBItem & operator = (const CDBItem & item);

	bool operator==(const CDBItem& item);

protected:
	unsigned char item[6];  //网络序存放，前4字节是地址，后2字节是端口
	unsigned int  intime;

};

#endif

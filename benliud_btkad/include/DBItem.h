/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

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
	unsigned char item[6];  //�������ţ�ǰ4�ֽ��ǵ�ַ����2�ֽ��Ƕ˿�
	unsigned int  intime;

};

#endif

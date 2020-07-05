// IPLocationParser.h: interface for the CIPLocationParser class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IP_LOCATION_PARSER_H
#define _IP_LOCATION_PARSER_H

#include <wx/wx.h>
#include <wx/file.h>
#include <string>

using namespace std;

class CIPLocationParser  
{
public:

	CIPLocationParser();
	virtual ~CIPLocationParser();
	bool	Parse(unsigned int iip, string& country, string& area);

protected:
	bool GetInfoByIndex(long index, wxFile& fp, string& country, string& area);
	long SearchIndex(unsigned long iip, wxFile& fp);
	unsigned long GetLongAddr(unsigned char *buf);
	void	GetArea(unsigned char* buffer, wxFile& fp, string& area);
	bool	GetStringByAddr(long addr, wxFile& fp, string& str);
};

#endif // !defined(AFX_IPLOCATIONPARSER_H__2C6F3EC4_396B_493D_B27A_31ACB7F65779__INCLUDED_)

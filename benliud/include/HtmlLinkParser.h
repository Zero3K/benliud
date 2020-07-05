// HtmlLinkParser.h: interface for the CHtmlLinkParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTMLLINKPARSER_H__C4BAE7AC_3EC6_4624_8610_A2E44FC1104C__INCLUDED_)
#define AFX_HTMLLINKPARSER_H__C4BAE7AC_3EC6_4624_8610_A2E44FC1104C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

using namespace std;
class CHtmlLinkParser  
{
public:
	std::string ToAbsoluteUrl(std::string& base, std::string& url);
	std::string GetUrl(int seq);
	int GetUrlCount();
	bool FindFramesetRange(const char *cbuf, int cfrom, int cend, int& begin, int& end);
	int Parse(std::string originuri,const char* filebuf,int filelen);
	CHtmlLinkParser(bool ignorepic=true);
	virtual ~CHtmlLinkParser();

protected:

	struct _ll
	{
		string note;
		string absuri;
	};

	vector<_ll> m_LinkList;
	bool m_bIgnorePic;
protected:
	void TrimAnchor(std::string& url);
	char* ToLower(char* str);
	bool FindKeywordValue(const char* cbuf,const char* key, char* value);
	bool FindBodyRange(const char *cbuf, int cfrom, int cend, int &begin, int &end);
	bool FindEndTag(const char *cbuf, int cfrom, int cend,char* tag ,int &pos);
	bool FindBeginTag(const char *cbuf, int cfrom, int cend,char* tag ,int &pos);
	bool FindHtmlRange(const char *cbuf, int cfrom, int cend, int& begin, int& end);
};

#endif // !defined(AFX_HTMLLINKPARSER_H__C4BAE7AC_3EC6_4624_8610_A2E44FC1104C__INCLUDED_)

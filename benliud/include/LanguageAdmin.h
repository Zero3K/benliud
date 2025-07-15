// LanguageAdmin.h: interface for the CLanguageAdmin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _LANGUAGEADMIN_H
#define _LANGUAGEADMIN_H

#include <string>
#include <wx/wx.h>
#include <wx/string.h>

#include <map>
#include <vector>

typedef std::map<std::string, wxString> TLanguageMap;

struct LangItem
{
	wxString LangName;
	wxString LangUuid;
	TLanguageMap LangMap;
};

class CLanguageAdmin  
{
public:
	wxString GetTranslator();
	wxString GetString(std::string id);
	void Init();
	CLanguageAdmin();
	virtual ~CLanguageAdmin();

protected:
	void LoadLanguage(wxString filename);

private:
	
	int m_nCurrentLangPos;

	std::vector<LangItem> m_AllLang;

	wxString m_Translator;
	//other language list
};

#endif

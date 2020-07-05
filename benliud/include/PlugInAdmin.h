/***************************************************************************
 *            CPlugInAdmin.h
 *
 *  Mon Nov 27 17:00:24 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _CPlugInAdmin_H
#define _CPlugInAdmin_H

#include "../include/msgtype_def.h"
#include "../include/protocol_def.h"
#include "../include/language_def.h"
#include "../include/module_def.h"
#include "../include/datatype_def.h"
#include "../include/callback_def.h"
#include "../include/modfun_def.h"
#include "../include/GlobalOption.h"

#include <wx/string.h>
#include <wx/dynlib.h>
#include <vector>

class CMsgCenter;
//non visiable class, admin and load the plugin.
class CPlugInAdmin
{
public:
	void StopService();
	void StartService();
	wxDynamicLibrary* GetServModule(_SERVICE_TYPE type);
	bool IsDuplicateUuid(wchar_t* uuid);
	CPlugInAdmin();
	virtual ~CPlugInAdmin();

	void Init();
	bool IsProtSupport(_PROT_TYPE type);
	bool IsLangSupport(_LANG_TYPE type);
	int  GetModuleNumber();
	bool GetModuleInfo(
			int seq,
			_MODULE_TYPE& type,
			_PROT_TYPE& ptype,
			wxString& uuid,
			wxString& author,
			wxString& version,
			wxString& desc);
	wxDynamicLibrary* GetProtModule(_PROT_TYPE type);
protected:
	bool IsLegalModuleType(_MODULE_TYPE type);	
	bool IsLegalUuid(wchar_t* uuid);

	struct _md
	{
		wxDynamicLibrary* pmod;
		wxString name;
	};

	//std::vector<wxDynamicLibrary*> m_ModList;
	std::vector<_md> m_ModList;

};

#endif

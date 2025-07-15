/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

���������GPL v2Э�鷢��.

****************************************************************/


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

//a simple file wrap for all platfrom
#ifndef _FILEWRAP_H
#define _FILEWRAP_H

#include "datatype_def.h"

#include <iostream>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#endif

class CFileWrap
{

public:
    CFileWrap();
    virtual ~CFileWrap();
    static llong GetFileLength( wchar_t* pathfile );
    static bool CreateEmptyFile( wchar_t* pathfile, llong len );

    //for read or write
    bool OpenFile( wchar_t* pathfile );

    //close file handle
    void CloseFile();

    //write data
    bool WriteFileData( llong pos, int dlen, void* data );

    bool ReadFileData( llong pos, int dlen, void* data );

protected:
    std::fstream m_filestream;

};

#endif


/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

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

#ifndef _HTTPURL_H
#define _HTTPURL_H

class CHttpUrl
{

public:
    CHttpUrl( char* url );
    bool IsOk();
    char* GetRawUrl()
    {
        return m_RawUrl;
    }

    char* GetServer()
    {
        return m_Server;
    }

    char* GetUser()
    {
        return m_User;
    }

    char* GetPass()
    {
        return m_Pass;
    }

    char* GetPathFile()
    {
        return m_PathFile;
    }

    char* GetFile()
    {
        return m_File;
    }

    char*	GetEscPathFile()
    {
        return m_EscPathFile;
    }

    char*	GetEscFile()
    {
        return m_EscFile;
    }

    int	GetPort()
    {
        return m_Port;
    }

    char*	GetEscUrl()
    {
        return m_EscUrl;
    }

protected:
    bool	CheckLen( char* str, int len );
    bool Parse( char* url );
    bool EscapePathFile();
    bool EscapeFile();
    bool Escape( char* from, char* to, int buflen );

    bool m_UrlOk;
    char m_RawUrl[ 256 ];
    char m_User[ 64 ];
    char m_Pass[ 64 ];
    char m_Server[ 64 ];
    int m_Port;
    char m_PathFile[ 256 ];
    char m_File[ 128 ];
    char m_EscPathFile[ 256 ];
    char m_EscFile[ 128 ];
    char m_EscUrl[ 256 ]; //the full escape url.
};

#endif


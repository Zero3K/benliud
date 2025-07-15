/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/


#include "stdafx.h"

#include "../include/HttpUrl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define TO_UPPER(x) ( ( (x)>='a' && (x)<='z' ) ? ((x)+'A'-'a'):(x))

#define XDIGIT_TO_XCHAR(x) (((x) < 10) ? ((x) + '0') : ((x) - 10 + 'A'))

#define ISXDIGIT(x) ( ((x) >= '0' && (x) <= '9')||\
	((x) >= 'a' && (x) <= 'z')||\
((x) >= 'A' && (x) <= 'Z') )

#define XCHAR_TO_XDIGIT(x) 	(((x) >= '0' && (x) <= '9') ? \
((x) - '0') : (TO_UPPER(x) - 'A' + 10))

//1 : reserved char
//2 : unsafe char
//3 : reserved and unsafe
const static unsigned char URL_CHAR_TABLE[ 256 ] =
    {
        2, 2, 2, 2, 2, 2, 2, 2,      /* NUL SOH STX ETX  EOT ENQ ACK BEL */
        2, 2, 2, 2, 2, 2, 2, 2,      /* BS  HT  LF  VT   FF  CR  SO  SI  */
        2, 2, 2, 2, 2, 2, 2, 2,      /* DLE DC1 DC2 DC3  DC4 NAK SYN ETB */
        2, 2, 2, 2, 2, 2, 2, 2,      /* CAN EM  SUB ESC  FS  GS  RS  US  */
        2, 0, 2, 3, 1, 2, 1, 0,      /* SP  !   "   #    $   %   &   '   */
        0, 0, 0, 1, 1, 0, 0, 1,      /* (   )   *   +    ,   -   .   /   */
        0, 0, 0, 0, 0, 0, 0, 0,      /* 0   1   2   3    4   5   6   7   */
        0, 0, 3, 1, 2, 1, 2, 1,      /* 8   9   :   ;    <   =   >   ?   */
        3, 0, 0, 0, 0, 0, 0, 0,      /* @   A   B   C    D   E   F   G   */
        0, 0, 0, 0, 0, 0, 0, 0,      /* H   I   J   K    L   M   N   O   */
        0, 0, 0, 0, 0, 0, 0, 0,      /* P   Q   R   S    T   U   V   W   */
        0, 0, 0, 3, 2, 3, 2, 0,      /* X   Y   Z   [    \   ]   ^   _   */
        2, 0, 0, 0, 0, 0, 0, 0,      /* `   a   b   c    d   e   f   g   */
        0, 0, 0, 0, 0, 0, 0, 0,      /* h   i   j   k    l   m   n   o   */
        0, 0, 0, 0, 0, 0, 0, 0,      /* p   q   r   s    t   u   v   w   */
        0, 0, 0, 2, 2, 2, 3, 2,      /* x   y   z   {    |   }   ~   DEL */

        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,

    };


#define URL_RESERVED_CHAR(c) (URL_CHAR_TABLE[(unsigned char)(c)]&1)
#define URL_UNSAFE_CHAR(c) (URL_CHAR_TABLE[(unsigned char)(c)]&2)

#ifdef _USE_SSL 
//https://
const char* _PREFIX = "HTTPS://";
const int _PREFIXLEN = 8;
const int _DEFAULTPORT = 443;
#else 
//http://
const char* _PREFIX = "HTTP://";
const int _PREFIXLEN = 7;
const int	_DEFAULTPORT = 80;
#endif

CHttpUrl::CHttpUrl( char* url )
{
    m_UrlOk = Parse( url );
}

bool CHttpUrl::IsOk()
{
    return m_UrlOk;
}

bool CHttpUrl::Parse( char* url )
{
    //http://userid:password@server.com:8890/path/to/file.txt
    //=>get raw url:ftp://server.com:8890/path/to/file.txt
    //=>get server: server.com
    //=>get port: 8890
    //=>get user: userid
    //=>get pass: password
    //=>get pathfile: /path/to/file.txt
    //=>get filename: file.txt

    //from front to find '@'
    //if not find the '@', it's a raw url.
    //if found '@', check if it in pathfile.

    //no matter how, first get the escape url
    //for sometime just need it.
    //check the url length

    if ( !CheckLen( url, 254 ) )
    {
        return false;	//too long uri
    }

    if ( !Escape( url, m_EscUrl, 256 ) )
    {
        return false; //too long uri
    }

#if defined( WIN32) || defined(WINCE)

    if ( _strnicmp( url, _PREFIX, _PREFIXLEN ) != 0 )

#else

    if ( strncasecmp( url, _PREFIX, _PREFIXLEN ) != 0 )

#endif

    {
        return false;
    }

    char* pmove = url;

    while ( *pmove != '@' && *pmove != '\0' )
    {
        pmove++;
    }

    if ( *pmove == '@' )
    { //check if between http://...and...@ have '/'
        char * pchk = url + _PREFIXLEN;

        while ( *pchk != '@' && *pchk != '/' )
            pchk++;

        if ( *pchk == '/' )
        {
            //@ is in pathfile, ignore it
            strcpy( m_RawUrl, url ); //raw url = url.
            m_Pass[ 0 ] = '\0'; //set default password
            m_User[ 0 ] = '\0'; //set default user.
        }
        else
        {
            //@ is a splitter
            char temp[ 256 ];
            //copy "userid:password" to temp.
            strncpy( temp, url + _PREFIXLEN, pmove - url - _PREFIXLEN ); //???
            temp[ pmove - url - _PREFIXLEN ] = '\0';
            strncpy( m_RawUrl, url, _PREFIXLEN );
            strcpy( m_RawUrl + _PREFIXLEN, pmove + 1 );
            char* psplit = temp;
            psplit++;

            while ( *psplit != ':' && *psplit != '\0' )
                psplit++;

            if ( *psplit == '\0' )
                return false; //no ':' found

            if ( strlen( temp ) - ( psplit - temp ) == 1 )
            { //':' at end
                //change 2006-12-14 pass maybe empty
                //check the user length

                if ( psplit - temp > 62 )
                    return false;

                //copy the username
                strncpy( m_User, temp, psplit - temp );

                m_Pass[ 0 ] = '\0';
            }
            else
            {
                //':' in middle, ok ,split user and pass.
                //check user length

                if ( psplit - temp > 62 )
                    return false;

                //copy the username
                strncpy( m_User, temp, psplit - temp );

                m_User[ psplit - temp ] = '\0';

                //check the pass length
                if ( strlen( psplit + 1 ) > 62 )
                    return false;

                strcpy( m_Pass, psplit + 1 );
            }
        }

    }
    else
    { //no '@' in url.
        strcpy( m_RawUrl, url ); //raw url = url.
        m_Pass[ 0 ] = '\0'; //set default password
        m_User[ 0 ] = '\0'; //set default user.
    }

    //do the left thing with m_RawUrl
    //the m_RawUrl should = http://server.com:8890/path/to/file.txt
    //find '/' after http://
    pmove = m_RawUrl + _PREFIXLEN;

    while ( *pmove != '/' && *pmove != '\0' )
        pmove++;

    if ( *pmove == '\0' )
    {
        //maybe it's a http://www.sina.com.cn:8890
        strcpy( m_PathFile, "/" );
        strcpy( m_File, "index.html" );

//        char* saveuse = pmove;
        //copy out "server.com:8890"
        char ser[ 70 ];
        //change 2006-12-14

        if ( pmove - m_RawUrl - _PREFIXLEN > 68 )
            return false; //server string too long

        //change 2007-01-23
        if ( pmove -m_RawUrl - _PREFIXLEN < 4 )
            return false; //server string too short

        strncpy( ser, m_RawUrl + _PREFIXLEN, pmove - m_RawUrl - _PREFIXLEN );

        ser[ pmove - m_RawUrl - _PREFIXLEN ] = '\0';

        //find server and port.
        pmove = ser + 1;

        while ( *pmove != ':' && *pmove != '\0' )
            pmove++;

        if ( *pmove == '\0' )
        { //no port
            //change 2006-12-14 check server length, m_Server have 64 chars

            if ( strlen( ser ) > 62 )
                return false;

            strcpy( m_Server, ser );

            m_Port = _DEFAULTPORT;
        }
        else
        { //have port

            if ( pmove - ser > 62 )
                return false; //change 2006-12-14 check server length

            strncpy( m_Server, ser, pmove - ser );

            m_Server[ pmove - ser ] = '\0';

            char port[ 10 ];

            if ( strlen( pmove + 1 ) > 5 )
                return false; //change 2006-12-14

            strcpy( port, pmove + 1 );

            if ( strlen( port ) == 0 )
                return false;

            m_Port = atoi( port ); //not check all digit

            if ( m_Port >= 65535 || m_Port <= 0 )
                return false;
        }

    }
    else
    {
        char* saveuse = pmove;
        //copy out "server.com:8890"
        char ser[ 70 ];
        //change 2006-12-14

        if ( pmove - m_RawUrl - _PREFIXLEN > 68 )
            return false; //server string too long

        //change 2007-01-23
        if ( pmove -m_RawUrl - _PREFIXLEN < 4 )
            return false; //server string too short

        strncpy( ser, m_RawUrl + _PREFIXLEN, pmove - m_RawUrl - _PREFIXLEN );

        ser[ pmove - m_RawUrl - _PREFIXLEN ] = '\0';

        //find server and port.
        pmove = ser + 1;

        while ( *pmove != ':' && *pmove != '\0' )
            pmove++;

        if ( *pmove == '\0' )
        { //no port
            //change 2006-12-14 check server length, m_Server have 64 chars

            if ( strlen( ser ) > 62 )
                return false;

            strcpy( m_Server, ser );

            m_Port = _DEFAULTPORT;
        }
        else
        { //have port

            if ( pmove - ser > 62 )
                return false; //change 2006-12-14 check server length

            strncpy( m_Server, ser, pmove - ser );

            m_Server[ pmove - ser ] = '\0';

            char port[ 10 ];

            if ( strlen( pmove + 1 ) > 5 )
                return false; //change 2006-12-14

            strcpy( port, pmove + 1 );

            if ( strlen( port ) == 0 )
                return false;

            m_Port = atoi( port ); //not check all digit

            if ( m_Port >= 65535 || m_Port <= 0 )
                return false;
        }

        //find pathfile and file.
        pmove = saveuse;

        //copy pathfile.
        strcpy( m_PathFile, pmove );

        //make out file. find last '/' in pathfile
        pmove = m_PathFile;

        pmove += strlen( m_PathFile );

        while ( *pmove != '/' )
            pmove--;

        if ( strlen( pmove + 1 ) > 62 )  //too long just for file name
        { //try to find '?' to trim the name
            char * ques = pmove + 1;

            while ( *ques != '?' && *ques != '\0' )
                ques++;

            if ( *ques == '\0' )
            {
                strncpy( m_File, pmove + 1, 63 );
                m_File[ 63 ] = '\0';
            }
            else
            {
                strncpy( m_File, pmove + 1, ques - pmove - 1 );
                m_File[ ques - pmove - 1 ] = '\0';
            }

        }
        else
        {
            strcpy( m_File, pmove + 1 );
        }

        if ( strlen( m_File ) == 0 )
            strcpy( m_File, "index.html" );

        if ( !EscapePathFile() )
        {
            return false;
        }

        if ( !EscapeFile() )
        {
            return false;
        }
    }

    return true;

}

bool CHttpUrl::Escape( char* from, char* to, int buflen )
{
    char * pori, *pptr;

    pori = from;

    pptr = to;

    while ( *pori != '\0' )
    {
        if ( *pori == '%' )
        {
            if ( pptr - to > buflen - 5 )
                return false; //check the buflen

            if ( ISXDIGIT( pori[ 1 ] ) && ISXDIGIT( pori[ 2 ] ) )
            {
                strncpy( pptr, pori, 3 );
                pori += 3;
                pptr += 3;
            }
            else
            {
                strncpy( pptr, "%25", 3 );
                pori += 1;
                pptr += 3;
            }
        }
        else if ( URL_UNSAFE_CHAR( *pori ) && !URL_RESERVED_CHAR( *pori ) )
        {
            if ( pptr - to > buflen - 5 )
                return false; //check the buflen

            pptr[ 0 ] = '%';

            pptr[ 1 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) >> 4 );

            pptr[ 2 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) & 0x0F );

            pori += 1;

            pptr += 3;
        }
        else
        {
            if ( pptr - to > buflen - 3 )
                return false; //check the buflen

            *pptr = *pori;

            pori += 1;

            pptr += 1;
        }
    } // end of while

    *pptr = '\0';

    return true;

}

bool CHttpUrl::EscapeFile()
{
    return Escape( m_File, m_EscFile, 128 );
}

bool CHttpUrl::EscapePathFile()
{
    return Escape( m_PathFile, m_EscPathFile, 256 );
}

//if str is more than the len return false
bool CHttpUrl::CheckLen( char *str, int len )
{
    for ( int i = 0;i < len;i++ )
    {
        if ( str[ i ] == '\0' )
            return true;
    }

    return false;
}

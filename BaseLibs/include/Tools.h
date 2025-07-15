//some cross platform utils

#ifndef _TOOLS_H
#define _TOOLS_H

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <string>


#define TO_UPPER(x) ( ( (x)>='a' && (x)<='z' ) ? ((x)+'A'-'a'):(x))

#define XDIGIT_TO_XCHAR(x) (((x) < 10) ? ((x) + '0') : ((x) - 10 + 'A'))

#define ISXDIGIT(x) ( ((x) >= '0' && (x) <= '9')||\
	((x) >= 'a' && (x) <= 'f')||\
((x) >= 'A' && (x) <= 'F') )

#define XCHAR_TO_XDIGIT(x) 	(((x) >= '0' && (x) <= '9') ? \
((x) - '0') : (TO_UPPER(x) - 'A' + 10))

#ifndef llong
	#ifdef WIN32
	typedef __int64 llong;
	#else
	typedef long long llong;
	#endif
#endif
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

namespace Tools
{

	std::string EscapeToString( unsigned char* src, int len);
	std::string EscapeToUrlString( unsigned char* src, int len);
	std::string UnEscapeFromUrlString( unsigned char* src, int len);
	std::string EscapeToUrlStringForName( unsigned char* src, int len);
	std::string EscapeFullUrl(std::string fullurl);
	std::string EscapeHash(unsigned char* src, int len);	


#if defined(WIN32) 
#include <wchar.h>
#endif

	int UCS2UTF( const wchar_t* ucs, char* utf, int bufleninbyte );
	int UTF2UCS( const char* utf, wchar_t* ucs, int bufleninword );	
	void PatchFileNameForWindows( wchar_t* name );
	bool parseUrl( const char* url, std::string& host, unsigned short* port, std::string& path );
	std::string SHA1String(const char* data, int datalen);
	std::string SHA1String(std::string& data);
	std::string MD5String(const char* data, int datalen);
	std::string MD5String(std::string data);
	void TrimString( std::string& str );	
	void TrimStringRight(std::string& str, const std::string &t);
	void TrimStringLeft(std::string& str, const std::string &t);
	void TrimAllSpace( std::string& str);
	void TrimAllSingleQuote( std::string& str);
	void TrimAllDoubleQuote( std::string& str);
	void ToUpper( std::string& str );
	void ToLower( std::string& str );
	bool compactaddr(std::string &ipport, std::string ip, unsigned short port);
	bool unpactaddr(std::string ipport, std::string& ip, unsigned short& port);
	void bin2txt(std::string& bin, std::string& txt);
	bool txt2bin(std::string& txt, std::string& bin);

#ifndef WIN32
	void Sleep(int ms);
	unsigned int GetTickCount();
#endif
}//namespace

#endif


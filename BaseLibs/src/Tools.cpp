
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

#include "../include/Tools.h"
#include "../include/SHA1.h"
#include "../include/MD5.h"
#include <algorithm>
#include <functional>

#ifdef WIN32

#else

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>  //inet_ntoa
#include <time.h>
#include <memory.h>  //for memcpy


unsigned int GetTickCount()
{
	static struct timeval now;
	gettimeofday( &now, NULL );
	return now.tv_sec*1000 + now.tv_usec / 1000;
}

#endif

namespace Tools
{

	//if src have substr like %AB also will convert to %25AB, so can be use to convert hash
	std::string EscapeHash(unsigned char* src, int len)
	{
		unsigned char *to=new unsigned char[len*3+2]; 

		unsigned char *pori, *pptr;

		pori = src;

		pptr = to;

		while ( pori - src - len < 0 )
		{
			if ( URL_UNSAFE_CHAR( *pori )||URL_RESERVED_CHAR(*pori) )
			{
				pptr[ 0 ] = '%';

				pptr[ 1 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) >> 4 );

				pptr[ 2 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) & 0x0F );

				pori += 1;

				pptr += 3;
			}
			else
			{

				*pptr = *pori;

				pori += 1;

				pptr += 1;
			}
		} // end of while

		*pptr = 0;

		std::string ret= std::string((char*)to);

		delete[] to;

		return ret;
	}

	//convert binary value to string to show, not for urls
	std::string EscapeToString( unsigned char* src, int len)
	{

		unsigned char *to=new unsigned char[len*3+2]; 

		unsigned char *pori, *pptr;

		pori = src;

		pptr = to;

		while ( pori - src - len < 0 )
		{
			if ( URL_UNSAFE_CHAR( *pori ) )
			{
				pptr[ 0 ] = '%';

				pptr[ 1 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) >> 4 );

				pptr[ 2 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) & 0x0F );

				pori += 1;

				pptr += 3;
			}
			else
			{

				*pptr = *pori;

				pori += 1;

				pptr += 1;
			}
		} // end of while

		*pptr = 0;

		std::string ret= std::string((char*)to);

		delete[] to;

		return ret;

	}

	std::string UnEscapeFromUrlString( unsigned char* src, int len)
	{

		unsigned char *to=new unsigned char[len+2]; 

		unsigned char *pori, *pptr;

		pori = src;

		pptr = to;

		while ( *pori )
		{
			if ( *pori == '%' )
			{
				if(ISXDIGIT( *(pori+1) ) && ISXDIGIT( *(pori+2) ) )
				{
					//merge the too char into one char
					*pptr = (XCHAR_TO_XDIGIT( *(pori+1) ) <<4 ) + XCHAR_TO_XDIGIT( *(pori+2) );
					pptr++;
					pori+=3;
				}
				else
				{//just copy it
					*pptr = *pori;
					pori ++;
					pptr ++;
				}

			}
			else
			{//just copy it
				*pptr = *pori;
				pori ++;
				pptr ++;
			}
		} // end of while

		*pptr = 0;

		std::string ret= std::string((char*)to);

		delete[] to;

		return ret;	
	}

	//only for convert hash to url legal string
	//if the string is alread escaped , don't call it again!
	//only escape file or dir name not full url like http://www.ssss.com/abc/sdfa/dfa.txt
	//if the string have substr like %AB , it will remain , so can't use to convert hash!!!
	std::string EscapeToUrlString( unsigned char* src, int len)
	{

		unsigned char *to = new unsigned char[ len * 3 + 2 ];

		unsigned char *pori, *pptr;

		pori = src;

		pptr = to;

		while ( pori - src - len < 0 )
		{
			if ( *pori=='%' && ISXDIGIT( *(pori+1) ) && ISXDIGIT( *(pori+2) ))
			{//alread in escape mode
				*pptr = *pori;
				*(pptr+1)=*(pori+1);
				*(pptr+2)=*(pori+2);			
				pori += 3;
				pptr += 3;
			}
			else if (URL_RESERVED_CHAR(*pori)||URL_UNSAFE_CHAR(*pori))
				//else if (URL_UNSAFE_CHAR(*pori))
			{
				pptr[ 0 ] = '%';
				pptr[ 1 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) >> 4 );
				pptr[ 2 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) & 0x0F );
				pori += 1;
				pptr += 3;
			}
			else
			{
				*pptr = *pori;
				pori += 1;
				pptr += 1;
			}

		} // end of while

		*pptr = '\0';

		std::string ret = std::string( (char*)to );

		delete[] to;

		return ret;
	}

	//only for the name part, maybe like this "modules.php?name=Site_Downloads&op=mydown&did=4827"
	//so should skip the & ? etc
	std::string EscapeToUrlStringForName( unsigned char* src, int len)
	{

		unsigned char *to = new unsigned char[ len * 3 + 2 ];

		unsigned char *pori, *pptr;

		pori = src;

		pptr = to;

		while ( pori - src - len < 0 )
		{
			if ( *pori=='%' && ISXDIGIT( *(pori+1) ) && ISXDIGIT( *(pori+2) ))
			{//alread in escape mode
				*pptr = *pori;
				*(pptr+1)=*(pori+1);
				*(pptr+2)=*(pori+2);			
				pori += 3;
				pptr += 3;
			}
			//else if (URL_RESERVED_CHAR(*pori)||URL_UNSAFE_CHAR(*pori))
			else if (URL_UNSAFE_CHAR(*pori))
			{
				pptr[ 0 ] = '%';
				pptr[ 1 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) >> 4 );
				pptr[ 2 ] = XDIGIT_TO_XCHAR( ( ( unsigned char ) ( *pori ) ) & 0x0F );
				pori += 1;
				pptr += 3;
			}
			else
			{
				*pptr = *pori;
				pori += 1;
				pptr += 1;
			}

		} // end of while

		*pptr = '\0';

		std::string ret = std::string( (char*)to );

		delete[] to;

		return ret;
	}

	std::string EscapeFullUrl(std::string fullurl)
	{
		//split the full url into file and dir part
		size_t head=fullurl.find("://");
		if(head==std::string::npos) return fullurl;

		//find the third '/'
		size_t third=fullurl.find('/',head+3);
		if(third==std::string::npos) return fullurl;

		std::string escaped=fullurl.substr(0,third); // the http://www.sohu.com

		//escape all the dir and filename one by one
		size_t mid1=third+1;
		size_t mid2=third+1;

		while((mid2=fullurl.find('/',mid1))!=std::string::npos)
		{
			std::string part=fullurl.substr(mid1,mid2-mid1);
			//escape it
			std::string escpart=EscapeToUrlString((unsigned char*)part.c_str(),part.length());
			escaped+="/";
			escaped+=escpart;
			mid1=mid2+1;
		}

		//escape the left filename
		std::string fn=fullurl.substr(mid1);
		//escape it;
		std::string escpart=EscapeToUrlStringForName((unsigned char*)fn.c_str(),fn.length());
		escaped+="/";
		escaped+=escpart;
		return escaped;

	}

	int UCS2UTF( const wchar_t* ucs, char* utf, int bufleninbyte )
	{
		if ( ucs == ( wchar_t* ) 0 || utf == ( char* ) 0 )
			return 0;

		const wchar_t* pmove = ucs;

		int counter = 0;

		while ( *pmove )
		{
			if ( ( *pmove & ~( 0x007F ) ) == ( wchar_t ) 0 )  //1 byte
			{ //0000 - 007F -> 0xxxxxxx

				if ( bufleninbyte - counter < 2 )
				{
					return -1;
				}

				*( utf + counter ) = ( char ) ( *pmove );
				++counter;
			}
			else if ( ( *pmove & ~( 0x07FF ) ) == ( wchar_t ) 0 )  //2 bytes
			{ //0080 - 07FF -> 110xxxxx 10xxxxxx

				if ( bufleninbyte - counter < 3 )
				{
					return -1;
				}


				*( utf + counter ) = ( char ) ( ( *pmove ) >> 6 ) + ( char ) 0xC0;
				++counter;
				*( utf + counter ) = ( char ) ( ( *pmove ) & 0x3F ) + ( char ) 0x80;
				++counter;
			}
			else //3 bytes
			{ //0800 - FFFF -> 1110xxxx 10xxxxxx 10xxxxxx

				if ( bufleninbyte - counter < 4 )
				{
					return -1;
				}

				*( utf + counter ) = ( char ) ( ( *pmove ) >> 12 ) + ( char ) 0xE0;
				++counter;
				*( utf + counter ) = ( char ) ( ( ( *pmove ) >> 6 ) & ( char ) 0x3F ) + ( char ) 0x80;
				++counter;
				*( utf + counter ) = ( char ) ( ( *pmove ) & ( char ) 0x3F ) + ( char ) 0x80;
				++counter;
			}

			pmove++;
		}

		*( utf + counter ) = '\0';
		return counter;
	}

	int UTF2UCS( const char* utf, wchar_t* ucs, int bufleninword )
	{
		//test for first byte to judge how may bytes should combine
		//0000 - 007F <- 0xxxxxxx
		//0080 - 07FF <- 110xxxxx 10xxxxxx
		//0800 - FFFF <- 1110xxxx 10xxxxxx 10xxxxxx

		if ( ucs == ( wchar_t* ) 0 || utf == ( char* ) 0 )
			return 0;

		const char* pmove = utf;

		int counter = 0;

		while ( *pmove )
		{
			if ( ( *pmove & 0x80 ) == '\0' )
			{ //0000 - 007F <- 0xxxxxxx

				if ( bufleninword - counter < 2 )
				{
					return -1;
				}

				*( ucs + counter ) = ( wchar_t ) ( *pmove );
				counter++;
			}
			else if ( ( *pmove & 0xE0 ) == 0xC0 )
			{ //0080 - 07FF <- 110xxxxx 10xxxxxx
				//this range not tested
				//comfirm the later byte is begin with 10

				if ( bufleninword - counter < 2 )
				{
					return -1;
				}

				if ( *( pmove + 1 ) == '\0' )
				{
					*( ucs + counter ) = '\0';
					return -2;  //data not in utf-8 format
				}

				if ( ( *( pmove + 1 ) & 0xC0 ) != 0x80 )
				{
					*( ucs + counter ) = '\0';
					return -2; //data not in utf-8 format
				}

				//check ok, convert data;
				char p = ( *( pmove ) & 0x1F ); //000xxxxx

				char q = ( *( pmove + 1 ) & 0x3F ); //00xxxxxx

				//combine char to word
				*( ucs + counter ) = wchar_t( ( ( wchar_t( p ) ) >> 2 ) << 8 ) +
					wchar_t( ( ( p & 0x03 ) << 6 ) + q );

				counter++;

				pmove++;
			}
			else
			{ //0800 - FFFF <- 1110xxxx 10xxxxxx 10xxxxxx

				if ( bufleninword - counter < 2 )
				{
					return -1;
				}

				if ( *( pmove + 1 ) == '\0' || *( pmove + 2 ) == '\0' )
				{
					*( ucs + counter ) = '\0';
					return -2; //data end, not in utf-8 format
				}

				//comfirm the later 2 bytes is begin with 10
				if ( ( *( pmove + 1 ) & 0xC0 ) != 0x80 || ( *( pmove + 2 ) & 0xC0 ) != 0x80 )
				{
					*( ucs + counter ) = '\0';
					return -2; //data not in utf-8 format
				}

				//check ok, convert data;

				*( ucs + counter ) = wchar_t( ( wchar_t( *( pmove ) & 0x0F ) ) << 12 ) +
					wchar_t( ( wchar_t( *( pmove + 1 ) & 0x3F ) ) << 6 ) +
					wchar_t( *( pmove + 2 ) & 0x3F );

				counter++;

				pmove += 2;

			}

			pmove++;
		}

		*( ucs + counter ) = '\0';
		return counter;
	}

	std::string SHA1String(std::string& data)
	{
		return SHA1String(data.data(),data.size());
	}

	std::string SHA1String(const char* data, int datalen)
	{
		HashLib::CSHA1 tmp;
		tmp.Hash(data, datalen);
		std::string sh;
		sh.append((const char*)tmp.GetHash(), tmp.GetHashLen());
		return sh;
		//unsigned char hash[20];
		//SHA1Block((unsigned char*)data,datalen,hash);
		//
		//std::string sh;
		//sh.append((char*)hash,20);
		//return sh;
	}

	std::string MD5String(std::string data)
	{
		//CMD5 md5;
		//std::string sh;
		//unsigned char* pbuf=md5.Encrypt( (unsigned char*)(data.data()), data.size() );
		//sh.append((char*)pbuf,16);
		//return sh;
		return MD5String(data.data(), data.size());
	}

	std::string MD5String(const char* data, int datalen)
	{
		HashLib::CMD5 tmp;
		tmp.Hash(data,datalen);
		std::string sh;
		sh.append((const char*)tmp.GetHash(), tmp.GetHashLen());
		return sh;
		//CMD5 md5;
		//std::string sh;
		//unsigned char* pbuf=md5.Encrypt( (unsigned char*)data, datalen );
		//sh.append((char*)pbuf,16);
		//return sh;

	}

	//trim space at head and rear
	void TrimString( std::string& str )
	{
		std::string::size_type pos = str.find_last_not_of( ' ' );

		if ( pos != std::string::npos )
		{
			str.erase( pos + 1 );
			pos = str.find_first_not_of( ' ' );

			if ( pos != std::string::npos )
				str.erase( 0, pos );
		}
		else
			str.erase( str.begin(), str.end() );
	}


	void TrimStringLeft(std::string& str, const std::string &t)
	{
		str.erase(0, str.find_first_not_of(t));
	}

	void TrimStringRight(std::string& str, const std::string &t)
	{
		str.erase(str.find_last_not_of(t)+1);
	}


	bool parseUrl( const char* url, std::string& host, unsigned short* port, std::string& path )
	{
		std::string str_url = url;

		std::string::size_type pos = str_url.find( "://" );

		if ( pos == std::string::npos )
		{
			return false;
		}

		str_url.erase( 0, pos + 3 );

		pos = str_url.find( ":" );

		if ( pos == std::string::npos )
		{
			*port = 80;
			pos = str_url.find( "/" );

			if ( pos == std::string::npos )
			{
				return false;
			}

			host = str_url.substr( 0, pos );

			str_url.erase( 0, pos );
		}
		else
		{
			host = str_url.substr( 0, pos );

			str_url.erase( 0, pos + 1 );

			pos = str_url.find( "/" );

			if ( pos == std::string::npos )
			{
				return false;
			}

			std::string str_port = str_url.substr( 0, pos );
			*port = ( unsigned short ) atoi( str_port.c_str() );

			str_url.erase( 0, pos );
		}

		if ( str_url.length() == 0 )
		{
			path = "/";
		}
		else
		{
			path = str_url;
		}

		return true;
	}

#ifndef WIN32
	void Sleep(int ms)
	{
		usleep(ms*1000);
	}
#endif

	void ToUpper( std::string& str )
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}
	void ToLower( std::string& str )
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
	}

	void TrimAllSpace( std::string& str)
	{
		str.erase(
			std::remove_if( str.begin(), str.end(), std::bind2nd( std::equal_to<char>(), ' ' ) ),
			str.end() );
	}

	void TrimAllSingleQuote( std::string& str)
	{
		str.erase(
			std::remove_if( str.begin(), str.end(), std::bind2nd( std::equal_to<char>(), '\'' ) ),
			str.end() );
	}

	void TrimAllDoubleQuote( std::string& str)
	{
		str.erase(
			std::remove_if( str.begin(), str.end(), std::bind2nd( std::equal_to<char>(), '\"' ) ),
			str.end() );
	}

	//some time the filename have illegal char for windows
	//this func will not fix the full path like c:\\abc\test.txt because it have :
	//this func only fix the path and name like \\abcd?:\filename?<>.txt
	//				m_SaveName.Replace(L"\\",L"_");
	//				m_SaveName.Replace(L"/",L"_");
	//				m_SaveName.Replace(L":",L"_");
	//				m_SaveName.Replace(L"*",L"_");
	//				m_SaveName.Replace(L"?",L"_");
	//				m_SaveName.Replace(L"\"",L"_");
	//				m_SaveName.Replace(L"<",L"_");
	//				m_SaveName.Replace(L">",L"_");
	//				m_SaveName.Replace(L"|",L"_");
	void PatchFileNameForWindows( wchar_t* name )
	{

		for(wchar_t *move=name; *move; move++)
		{
			if( *move==L'\\'||
				*move==L'/' ||
				*move==L':' ||
				*move==L'*' ||
				*move==L'?' ||
				*move==L'\"'||
				*move==L'<' ||
				*move==L'>' ||
				*move==L'|' )
			{
				*move=L'_';
			}

		}
	}

	////convert bitset from binary format to text format
	//std::string bin2text(std::string bin)
	//{
	//	char *buf=new char[bin.size()*2+2];

	//	for(unsigned int i=0;i<bin.size();i++)
	//	{
	//		unsigned char t= bin[i];

	//		buf[i*2] = XDIGIT_TO_XCHAR(  t >> 4 );

	//		buf[i*2+1] = XDIGIT_TO_XCHAR( t & 0x0F );
	//	}

	//	buf[bin.size()*2]=0;
	//	std::string text=buf;

	//	delete[] buf;
	//	return text;
	//}

	void bin2txt(std::string& bin, std::string& txt)
	{
		for(unsigned int i=0;i<bin.size();i++)
		{
			txt.append(1, XDIGIT_TO_XCHAR( ((( unsigned char) bin[i]) &0xF0) >> 4 )); //shift should be unsigned
			txt.append(1, XDIGIT_TO_XCHAR( ((unsigned char) bin[i]) & 0x0F ));
		}
	}

	bool txt2bin(std::string& txt, std::string& bin)
	{

		if(txt.size()%2) return false;

		for(unsigned int i=0;i<txt.size();i+=2)
		{
			bin.append(1, char(( ((unsigned char)XCHAR_TO_XDIGIT( txt[i] )) <<4 ) + (unsigned char)XCHAR_TO_XDIGIT( txt[i+1] )));
		}

		return true;
	}

	//std::string text2bin(std::string text)
	//{
	//	char buf[1];

	//	if(text.size()%2!=0) return "";

	//	std::string bin;
	//	for(int i=0;i<text.size();i+=2)
	//	{
	//		char a=text[i];
	//		char b=text[i+1];

	//		buf[0]= char((XCHAR_TO_XDIGIT( a ) <<4 ) + XCHAR_TO_XDIGIT( b ));
	//		bin.append(buf,1);
	//	}

	//	return bin;
	//}

	//compact to 6 bytes 
	bool compactaddr(std::string &ipport, std::string ip, unsigned short port)
	{
		//compact the addr to 6 bytes net-order stream
		unsigned int a=inet_addr(ip.c_str()); //is net byte order
		if(a==INADDR_NONE) return false;

		unsigned short p=htons(port);

		char buf[6];
		memcpy(buf,&a,4);
		memcpy(buf+4,&p,2);

		ipport.resize(0);
		ipport.append(buf,6);
		return true;
	}

	bool unpactaddr(std::string ipport, std::string& ip, unsigned short& port)
	{
		if(ipport.size()!=6) return false;

		unsigned int iip = *( ( unsigned int* ) ( ipport.data() ) );
		port = *( ( unsigned short* ) ( ipport.data() + 4 ) );
		port = ntohs( port );



#ifdef WIN32
		char *ret;
		ret = inet_ntoa( *((in_addr*)(&iip)) );
#else
		const char *ret;
		char ipbuf[ INET_ADDRSTRLEN ];
		ret = inet_ntop( AF_INET, (const void*)(&iip), ipbuf, INET_ADDRSTRLEN );
#endif

		if(ret==NULL) return false;
		ip=ret;
		return true;
	}

}

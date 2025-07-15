/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/

#include "stdafx.h"

#include "../include/filewrap.h"
#include <Tools.h>


CFileWrap::CFileWrap()
{
}

CFileWrap::~CFileWrap()
{ //close file
    CloseFile();
}

//if file not exist return -1
llong CFileWrap::GetFileLength( wchar_t* pathfile )
{
#if defined (WIN32)||defined(__WXMSW__)||defined(WINCE)
    //use sdk function to open the file directly
	HANDLE fh = ::CreateFile( pathfile, READ_CONTROL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL );

    if ( fh == INVALID_HANDLE_VALUE )
    { //file can't be open, not exists
        return -1;
    }


    DWORD high;
    DWORD low = GetFileSize( fh, &high );

    if ( low != INVALID_FILE_SIZE )
    {
        CloseHandle( fh );
        llong fsize = ( llong ) high;
        fsize <<= 32;
        fsize += ( llong ) low;
        return fsize;
    }
    else
    {
        CloseHandle( fh );

        return -1;
    }

#else

    //this code seems no ok in windows with large size file.

    char utf[ 512 ];

    int nret = Tools::UCS2UTF( pathfile, utf, 512 );

    if ( nret <= 0 )
        return -1;

    std::ifstream f;

    f.open( utf, std::ios::binary | std::ios::in );

    if ( !f.good() || f.eof() || !f.is_open() )
    {
        return -1;
    }

    f.seekg( 0, std::ios::beg );
    std::ifstream::pos_type begin_pos = f.tellg();
    f.seekg( 0, std::ios::end );
    llong len = static_cast<llong>( f.tellg() - begin_pos );
    f.close();
    return len;

#endif

}

//create a new file and keep the handle
bool CFileWrap::CreateEmptyFile( wchar_t* pathfile, llong len )
{

#if defined( WIN32)||defined(WINCE)

    HANDLE fileHandle;
    fileHandle = CreateFile(
                     pathfile,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL );

    if ( INVALID_HANDLE_VALUE == fileHandle )
        return false;

    if ( len < 0 )
        len = 0; //len==-1 is for create a empty file

	LARGE_INTEGER li;

    li.QuadPart = len;

	li.LowPart =SetFilePointer(
					fileHandle,
					li.LowPart,
					&li.HighPart,
					FILE_BEGIN
					);

	if(li.LowPart == ((DWORD)-1) && GetLastError()!=NO_ERROR)
	{
		return false;
	}

	if(!SetEndOfFile(fileHandle))
	{
		return false;
	}


	CloseHandle(fileHandle);
	return true;

#else

    if ( len <= 0 )
    {
        char utf[ 512 ];
        int nret = Tools::UCS2UTF( pathfile, utf, 512 );

        if ( nret <= 0 )
        {
            return false;
        }


        std::ofstream sfile( utf, std::ios::binary | std::ios::out | std::ios::trunc );

        if ( sfile.bad() || !sfile.is_open() || sfile.fail() )
        {
            return false;
        }

        sfile.close();
        return true;
    }

    char utf[ 512 ];
    int nret = Tools::UCS2UTF( pathfile, utf, 512 );

    if ( nret <= 0 )
    {
        return false;
    }

    FILE *fp;

    if ( NULL != ( fp = fopen( utf, "w" ) ) )
    {
#if defined(__BSD__)|| defined(__DARWIN__)	//it's wx preprocessor
        //bsd and mac haven't ftruncate64

        if ( 0 != ftruncate( fileno( fp ), len ) )
#else //linux have ftruncate64

    if ( 0 != ftruncate64( fileno( fp ), len ) )
#endif

        {
            return false;
        }
        else
        {
            fclose( fp );
            return true;
        }

    }
    else
    {
        return false;
    }

#endif

}

//for read or write
bool CFileWrap::OpenFile( wchar_t* pathfile )
{
    if ( m_filestream.is_open() )
    {
        m_filestream.close();
    }

    char utf[ 512 ];
    int nret = Tools::UCS2UTF( pathfile, utf, 512 );

    if ( nret <= 0 )
        return false;

    m_filestream.open( utf, std::ios_base::binary | std::ios_base::in | std::ios_base::out );

    if ( !m_filestream.is_open() )
        return false;

    return true;
}

//close file handle
void CFileWrap::CloseFile()
{
    if ( m_filestream.is_open() )
    {
        m_filestream.close();
    }
}

//write data
bool CFileWrap::WriteFileData( llong pos, int dlen, void* data )
{

    m_filestream.clear();
    //if(!m_filestream.is_open()) return false;
    m_filestream.seekp( pos, std::ios_base::beg );
    //if(m_filestream.fail()) return false;
    m_filestream.write( ( const char* ) data, dlen );
    //if(m_filestream.fail()) return false;
    m_filestream.flush();

    return m_filestream.good();


}

//read data
//dlen is bytes want to read
//data is buffer for read.
bool CFileWrap::ReadFileData( llong pos, int dlen, void* data )
{
    if ( !m_filestream.is_open() )
        return false;

    m_filestream.seekg( pos, std::ios_base::beg );

    if ( m_filestream.fail() )
        return false;

    m_filestream.read( ( char* ) data, dlen );

    if ( m_filestream.fail() )
        return false;

    return true;
}

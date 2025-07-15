/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/

#ifndef _TFILEINFO_H
#define _TFILEINFO_H

#include <string>

#ifndef llong
#ifdef WIN32
typedef __int64 llong;
#else
typedef long long int llong;
#endif
#endif

namespace BencodeLib
{
	class TFileInfo
	{
	public:
		int		index;
		bool	vfile; //virtual file, all zero, don't display, don't download
		//begin with "___padding_file" is bc virtual file
		//begin with "___virtual_file" is reserved by benliud
		llong	offset;
		llong	size;
		std::string name;
	};
}
#endif

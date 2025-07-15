/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

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

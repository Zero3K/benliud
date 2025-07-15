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

class TFileInfo
{
public:
	int		fileIndex;
	llong	offset;
	llong	size;
	std::string path;
	std::string name;
	std::string pathUTF8;
	std::string nameUTF8;	
};

#endif

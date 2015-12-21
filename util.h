
#ifndef _UTIL_H_
#define _UTIL_H_

#include<cstring>

inline std::string strlower(const std::string &s)
{
	int l = s.length();
	std::string sl = s;
	for (int i = 0; i < l; i++)
		sl[i] = tolower(sl[i]);
	return sl;
}

#endif // _UTIL_H_

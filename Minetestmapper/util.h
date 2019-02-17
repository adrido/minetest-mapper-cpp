#pragma once

#include <cstring>

inline std::string strlower(const std::string &s)
{
	size_t l = s.length();
	std::string sl = s;
	for (size_t i = 0; i < l; i++)
		sl[i] = tolower(sl[i]);
	return sl;
}

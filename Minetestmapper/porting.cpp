#include "porting.h"

#include <cstdio> // fopen
#include <cstdlib> // getenv
#include <cstring>



inline void sleepMs(int time)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

FILE *porting::fopen(const char *filename, const char *mode)
{
	FILE *file = nullptr;
#ifdef _WIN32	
	fopen_s(&file, filename, mode);
#else
	file = ::fopen(filename, mode);
#endif // _WIN32

	return file;
}

std::string porting::getenv(const char *name)
{
	std::string env;
	char *buf = nullptr;

#ifdef _WIN32
	std::size_t len;
	_dupenv_s(&buf, &len, name);
#else
	env = ::getenv(name);
#endif // _WIN32
	return buf == nullptr ? std::string() : std::string(buf);
}

std::string porting::strerror(int errnum) 
{
#ifdef _WIN32
	const std::size_t len = 100;
	char errmsg[len];
	strerror_s(errmsg, len, errnum);
#else
	char *errmsg = std::strerror(errnum);
#endif // _WIN32

	return std::string(errmsg);
}

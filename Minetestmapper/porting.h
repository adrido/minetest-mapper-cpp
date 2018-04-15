#pragma once

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#ifdef _MSC_VER
#ifndef strcasecmp 
#define strcasecmp(a, b) _stricmp(a, b)
#endif
#endif

inline void sleepMs(int time);

namespace porting {

	/* 
	Wrapper for fopen_s on Windows. This is not only to keep the deprecation notice quit,
	it is also much faster the the deprecated fopen (~50 ms!)
	On other systems it does still use the original fopen
	*/
	FILE* fopen(const char *filename, const char *mode);

	/*
	Wrapper for getenv_s on Windows.
	Uses getenv on other OS.
	*/
	std::string getenv(const char *name);

	std::string strerror(int errnum);

}  // namespace porting


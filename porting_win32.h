
#include <windows.h>

#define sleepMs(x) Sleep(x)

inline uint64_t getRelativeTimeStampMs()
{
	return GetTickCount64();
}


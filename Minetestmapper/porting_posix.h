
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include <sys/time.h>

#define sleepMs(x) usleep((x)*1000)

inline uint64_t getRelativeTimeStampMs()
{
	int rv = -1;
	struct timespec ts;
	#ifdef CLOCK_MONOTONIC_RAW
	if (rv == -1)
		rv = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	#endif
	#ifdef _POSIX_MONOTONIC_CLOCK
	if (rv == -1)
		rv = clock_gettime(CLOCK_MONOTONIC, &ts);
	#endif
	if (rv == -1) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}
	else {
		return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	}
}


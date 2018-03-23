#ifndef _PORTING_H
#define _PORTING_H

#ifdef _WIN32
#include "porting_win32.h"
#else
#include "porting_posix.h"
#endif

#ifdef _MSC_VER

#define strcasecmp(a, b) _stricmp(a, b)

#endif

#endif // _PORTING_H


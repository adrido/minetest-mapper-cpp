
#pragma once

#include "build_config.h"


#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

// List of possible database names (for usage message)
#ifdef USE_SQLITE3
#define USAGE_NAME_SQLITE "/sqlite3"
#else
#define USAGE_NAME_SQLITE
#endif

#ifdef USE_POSTGRESQL
#define USAGE_NAME_POSTGRESQL "/postgresql"
#else
#define USAGE_NAME_POSTGRESQL
#endif

#ifdef USE_LEVELDB
#define USAGE_NAME_LEVELDB "/leveldb"
#else
#define USAGE_NAME_LEVELDB
#endif

#ifdef USE_REDIS
#define USAGE_NAME_REDIS "/redis"
#else
#define USAGE_NAME_REDIS
#endif

#define USAGE_DATABASES "auto" USAGE_NAME_SQLITE USAGE_NAME_POSTGRESQL USAGE_NAME_LEVELDB USAGE_NAME_REDIS

#define BLOCK_SIZE		16
#define MAPBLOCK_MIN		(-2048)
#define MAPBLOCK_MAX		2047
#define CHUNK_SIZE_DEFAULT	5

// Max number of node name -> color mappings stored in a mapblock
#define MAPBLOCK_MAXCOLORS	65536

// default database to use
#if defined(USE_SQLITE3) && !defined(USE_POSTGRESQL) && !defined(USE_LEVELDB) && !defined(USE_REDIS)
#define DEFAULT_BACKEND "sqlite3"
#elif !defined(USE_SQLITE3) && defined(USE_POSTGRESQL) && !defined(USE_LEVELDB) && !defined(USE_REDIS)
#define DEFAULT_BACKEND "postgresql"
#elif !defined(USE_SQLITE3) && !defined(USE_POSTGRESQL) && defined(USE_LEVELDB) && !defined(USE_REDIS)
#define DEFAULT_BACKEND "leveldb"
#elif !defined(USE_SQLITE3) && !defined(USE_POSTGRESQL) && !defined(USE_LEVELDB) && defined(USE_REDIS)
#define DEFAULT_BACKEND "redis"
#else
#define DEFAULT_BACKEND "auto"
#endif

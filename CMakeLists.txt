# CMakeList.txt: CMake-Projektdatei der obersten Ebene. Führen Sie eine globale Konfiguration durch,
# und schließen Sie hier Unterprojekte ein.
#
cmake_minimum_required (VERSION 3.8)

#find_package(Git)
#set (VERSION_UPDATE_FROM_GIT ON)
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#include ("GetVersionFromGitTag")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project (
	"Minetestmapper" 
	VERSION 0.1.0
	DESCRIPTION "Map generator for Minetest"
	LANGUAGES C CXX)

# Set output directories
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

OPTION(ENABLE_SQLITE3 "Enable sqlite3 backend" True)
OPTION(ENABLE_POSTGRESQL "Enable postgresql backend")
OPTION(ENABLE_LEVELDB "Enable LevelDB backend")
OPTION(ENABLE_REDIS "Enable redis backend")

# Find sqlite3
if(ENABLE_SQLITE3)
	find_library(SQLITE3_LIBRARY sqlite3)
	find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
	message (STATUS "sqlite3 library: ${SQLITE3_LIBRARY}")
	message (STATUS "sqlite3 headers: ${SQLITE3_INCLUDE_DIR}")
	if(SQLITE3_LIBRARY AND SQLITE3_INCLUDE_DIR)
		set(USE_SQLITE3 1)
		message(STATUS "sqlite3 backend enabled")
		include_directories(${SQLITE3_INCLUDE_DIR})
	else(SQLITE3_LIBRARY AND SQLITE3_INCLUDE_DIR)
		set(USE_SQLITE3 0)
		if(ENABLE_SQLITE3)
			message(SEND_ERROR "sqlite3 backend requested but sqlite3 libraries not found!")
		else(ENABLE_SQLITE3)
			message(STATUS "sqlite3 not enabled (sqlite3 libraries and/or headers not found)")
		endif(ENABLE_SQLITE3)
	endif(SQLITE3_LIBRARY AND SQLITE3_INCLUDE_DIR)
endif(ENABLE_SQLITE3)

# Find postgresql
if(ENABLE_POSTGRESQL)
	find_library(POSTGRESQL_LIBRARY NAMES pq libpq)
	find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h PATH_SUFFIXES postgresql)
	message (STATUS "postgresql library: ${POSTGRESQL_LIBRARY}")
	message (STATUS "postgresql headers: ${POSTGRESQL_INCLUDE_DIR}")
	if(POSTGRESQL_LIBRARY AND POSTGRESQL_INCLUDE_DIR)
		set(USE_POSTGRESQL 1)
		message(STATUS "postgresql backend enabled")
		include_directories(${POSTGRESQL_INCLUDE_DIR})
	else(POSTGRESQL_LIBRARY AND POSTGRESQL_INCLUDE_DIR)
		set(USE_POSTGRESQL 0)
		if(ENABLE_POSTGRESQL)
			message(SEND_ERROR "postgresql backend requested but postgresql libraries not found!")
		else(ENABLE_POSTGRESQL)
			message(STATUS "postgresql not enabled (postgresql libraries and/or headers not found)")
		endif(ENABLE_POSTGRESQL)
	endif(POSTGRESQL_LIBRARY AND POSTGRESQL_INCLUDE_DIR)
endif(ENABLE_POSTGRESQL)

# Find leveldb
if(ENABLE_LEVELDB)
	find_library(LEVELDB_LIBRARY NAMES leveldb libleveldb)
	find_path(LEVELDB_INCLUDE_DIR db.h PATH_SUFFIXES leveldb)
	message (STATUS "LevelDB library: ${LEVELDB_LIBRARY}")
	message (STATUS "LevelDB headers: ${LEVELDB_INCLUDE_DIR}")
	if(LEVELDB_LIBRARY AND LEVELDB_INCLUDE_DIR)
		set(USE_LEVELDB 1)
		message(STATUS "LevelDB backend enabled")
		include_directories(${LEVELDB_INCLUDE_DIR})
	else(LEVELDB_LIBRARY AND LEVELDB_INCLUDE_DIR)
		set(USE_LEVELDB 0)
		if(ENABLE_LEVELDB)
			message(SEND_ERROR "leveldb backend requested but leveldb libraries not found!")
		else(ENABLE_LEVELDB)
			message(STATUS "leveldb not enabled (leveldb libraries and/or headers not found)")
		endif(ENABLE_LEVELDB)
	endif(LEVELDB_LIBRARY AND LEVELDB_INCLUDE_DIR)
endif(ENABLE_LEVELDB)

# Find redis
if(ENABLE_REDIS OR ENABLE_ANY_DATABASE OR ENABLE_ALL_DATABASES)
	find_library(REDIS_LIBRARY hiredis)
	find_path(REDIS_INCLUDE_DIR hiredis.h PATH_SUFFIXES hiredis)
	message (STATUS "redis library: ${REDIS_LIBRARY}")
	message (STATUS "redis headers: ${REDIS_INCLUDE_DIR}")
	if(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
		set(USE_REDIS 1)
		message(STATUS "redis backend enabled")
		include_directories(${REDIS_INCLUDE_DIR})
	else(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
		set(USE_REDIS 0)
		if(ENABLE_REDIS OR ENABLE_ALL_DATABASES)
			message(SEND_ERROR "redis backend requested but redis libraries not found!")
		else(ENABLE_REDIS OR ENABLE_ALL_DATABASES)
			message(STATUS "redis not enabled (redis libraries and/or headers not found)")
		endif(ENABLE_REDIS OR ENABLE_ALL_DATABASES)
	endif(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
endif(ENABLE_REDIS OR ENABLE_ANY_DATABASE OR ENABLE_ALL_DATABASES)

# Schließen Sie Unterprojekte ein.
#add_subdirectory ("wingetopt")
add_subdirectory ("Minetestmapper")

message(STATUS "VCPKG_APPLOCAL_DEPS: ${VCPKG_APPLOCAL_DEPS}")

if(NOT PROJECT_VERSION_TWEAK)
	set(PROJECT_VERSION_TWEAK 0)
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/version.h.in"
	"${PROJECT_SOURCE_DIR}/Minetestmapper/version.h"
)
configure_file(
	"${PROJECT_SOURCE_DIR}/build_config.h.in"
	"${PROJECT_SOURCE_DIR}/Minetestmapper/build_config.h"
)



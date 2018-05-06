# CMakeList.txt: CMake-Projektdatei der obersten Ebene. Führen Sie eine globale Konfiguration durch,
# und schließen Sie hier Unterprojekte ein.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (
	"Minetestmapper"
	DESCRIPTION "Map generator for Minetest"
	LANGUAGES C CXX)

find_package(Git REQUIRED)

execute_process(
    COMMAND           "${GIT_EXECUTABLE}" describe --long --dirty=-WIP --tags
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE   git_result
    OUTPUT_VARIABLE   git_tag
    ERROR_VARIABLE    git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )
if( NOT git_result EQUAL 0 )
    message( FATAL_ERROR "Failed to execute Git: ${git_error}" )
endif()

if( git_tag MATCHES "([0-9]+).([0-9]+).([0-9]+)-?([0-9]+)?-g([0-9,a-f]+)" )
    set( git_version_major           "${CMAKE_MATCH_1}" )
    set( git_version_minor           "${CMAKE_MATCH_2}" )
    set( git_version_patch           "${CMAKE_MATCH_3}" )
    set( git_commits_since_last_tag  "${CMAKE_MATCH_4}" )
    set( git_hash                    "${CMAKE_MATCH_5}" )
else()
    message( FATAL_ERROR "Git tag isn't valid semantic version: [${git_tag}]" )
endif()

set (PROJECT_VERSION_MAJOR ${git_version_major})
set (PROJECT_VERSION_MINOR ${git_version_minor})
set (PROJECT_VERSION_PATCH ${git_version_patch})
set (PROJECT_VERSION_TWEAK ${git_commits_since_last_tag})
if(NOT PROJECT_VERSION_TWEAK)
	set(PROJECT_VERSION_TWEAK 0)
endif()

set (PROJECT_VERSION "${git_version_major}.${git_version_minor}.${git_version_patch}.${git_commits_since_last_tag}-g${git_hash}")
message(STATUS "Version: ${PROJECT_VERSION}")

# Set output directories
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

OPTION(ENABLE_SQLITE3 "Enable sqlite3 backend" True)
OPTION(ENABLE_POSTGRESQL "Enable postgresql backend")
OPTION(ENABLE_LEVELDB "Enable LevelDB backend")
OPTION(ENABLE_REDIS "Enable redis backend")

macro(EnableDBBackend NAME_TECHNICAL DATABASE_NAME)
	message (STATUS "${DATABASE_NAME} library: ${${NAME_TECHNICAL}_LIBRARY}")
	message (STATUS "${DATABASE_NAME} headers: ${${NAME_TECHNICAL}_INCLUDE_DIR}")
	if(${NAME_TECHNICAL}_LIBRARY AND ${NAME_TECHNICAL}_INCLUDE_DIR)
		set(USE_${NAME_TECHNICAL} 1)
		message(STATUS "${DATABASE_NAME} backend enabled")
		include_directories(${${NAME_TECHNICAL}_INCLUDE_DIR})
	else(${NAME_TECHNICAL}_LIBRARY AND ${NAME_TECHNICAL}_INCLUDE_DIR)
		set(USE_${NAME_TECHNICAL} 0)
		if(ENABLE_${NAME_TECHNICAL})
			message(SEND_ERROR "${DATABASE_NAME} backend requested but ${DATABASE_NAME} libraries not found!")
		else(ENABLE_${NAME_TECHNICAL})
			message(STATUS "${DATABASE_NAME} not enabled (${DATABASE_NAME} libraries and/or headers not found)")
		endif(ENABLE_${NAME_TECHNICAL})
	endif(${NAME_TECHNICAL}_LIBRARY AND ${NAME_TECHNICAL}_INCLUDE_DIR)
endmacro()

# Find sqlite3
if(ENABLE_SQLITE3)
	find_library(SQLITE3_LIBRARY sqlite3)
	find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
	EnableDBBackend(SQLITE3 sqlite3)
endif(ENABLE_SQLITE3)

# Find postgresql
if(ENABLE_POSTGRESQL)
	find_library(POSTGRESQL_LIBRARY NAMES pq libpq)
	find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h PATH_SUFFIXES postgresql)
	EnableDBBackend(POSTGRESQL postgresql)
endif(ENABLE_POSTGRESQL)

# Find leveldb
if(ENABLE_LEVELDB)
	find_library(LEVELDB_LIBRARY NAMES leveldb libleveldb)
	find_path(LEVELDB_INCLUDE_DIR db.h PATH_SUFFIXES leveldb)
	EnableDBBackend(LEVELDB LevelDB)
endif(ENABLE_LEVELDB)

# Find redis
if(ENABLE_REDIS OR ENABLE_ANY_DATABASE OR ENABLE_ALL_DATABASES)
	find_library(REDIS_LIBRARY hiredis)
	find_path(REDIS_INCLUDE_DIR hiredis.h PATH_SUFFIXES hiredis)
	EnableDBBackend(REDIS redis)
endif(ENABLE_REDIS OR ENABLE_ANY_DATABASE OR ENABLE_ALL_DATABASES)

# Schließen Sie Unterprojekte ein.
#add_subdirectory ("wingetopt")
add_subdirectory ("Minetestmapper")

message(STATUS "VCPKG_APPLOCAL_DEPS: ${VCPKG_APPLOCAL_DEPS}")

configure_file(
	"${PROJECT_SOURCE_DIR}/version.h.in"
	"${PROJECT_SOURCE_DIR}/Minetestmapper/version.h"
)
configure_file(
	"${PROJECT_SOURCE_DIR}/build_config.h.in"
	"${PROJECT_SOURCE_DIR}/Minetestmapper/build_config.h"
)



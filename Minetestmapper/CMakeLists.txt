# CMakeList.txt: CMake-Projekt für "Minetestmapper". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 11)

set(sources
    PixelAttributes.cpp
	PixelAttributes.h
	PlayerAttributes.cpp
	PlayerAttributes.h
	TileGenerator.cpp
	TileGenerator.h
	ZlibDecompressor.cpp
	ZlibDecompressor.h
	Color.cpp
	Color.h
	Settings.cpp
	Settings.h
	BlockPos.cpp
	BlockPos.h
	mapper.cpp
	CharEncodingConverter.cpp
	CharEncodingConverter.h
	PaintEngine_libgd.cpp
	PaintEngine_libgd.h
	db.h
	db-leveldb.cpp
	db-leveldb.h
	db-postgresql.cpp
	db-postgresql.h
	db-redis.cpp
	db-redis.h
	db-sqlite3.cpp
	db-sqlite3.h
)

if(WIN32)
set(sources ${sources} ResTempl1.rct)
add_definitions(-DVER_COMPANYNAME_STR="MyCompany")
add_definitions(-DVER_FILEVERSION_STR="1,1,0.0")
endif(WIN32)

find_package (sqlite3 REQUIRED)
find_package (zlib REQUIRED)

find_library(LIBGD_LIBRARY NAMES libgd libgd_static)
find_path(LIBGD_INCLUDE_DIR NAMES gd.h)


include_directories(../wingetopt/src/)
find_path(SYSTEM_INCLUDE_DIR dirent.h)
include_directories(${SYSTEM_INCLUDE_DIR})

add_library (wingetopt STATIC ../wingetopt/src/getopt.c)
#link_directories (../wingetopt)
# Fügen Sie der ausführbaren Datei für dieses Projekt eine Quelle hinzu.
add_executable (Minetestmapper ${sources})
target_link_libraries(Minetestmapper wingetopt ${LIBGD_LIBRARY} ${ZLIB_LIBRARY} sqlite3)
# TODO: Fügen Sie bei Bedarf Tests und Installationsziele hinzu.


set (CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
include (InstallRequiredSystemLibraries)
include (CPack)
set(CPACK_GENERATOR "ZIP")
install (TARGETS Minetestmapper RUNTIME DESTINATION ".")

# Config might not set, so there might missing dll's after installation
install(DIRECTORY "${PROJECT_BINARY_DIR}/Minetestmapper/$<CONFIG>/" DESTINATION "." 
	FILES_MATCHING PATTERN "*.dll" PATTERN "/" EXCLUDE)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/colors/" DESTINATION "colors/" 
	FILES_MATCHING PATTERN "*.txt")
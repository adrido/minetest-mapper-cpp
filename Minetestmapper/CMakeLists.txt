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
	DataFileParser.cpp
	DataFileParser.h
	Settings.cpp
	Settings.h
	BlockPos.cpp
	BlockPos.h
	MapBlock.cpp
	MapBlock.h
	mapper.cpp
	mapper.h
	main.cpp
	CharEncodingConverter.cpp
	CharEncodingConverter.h
	PaintEngine.h
	PaintEngine_libgd.cpp
	PaintEngine_libgd.h
	PaintEngine_libgdTTF.cpp
	PaintEngine_libgdTTF.h
	porting.cpp
	porting.h
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
set(sources ${sources} Minetestmapper.rc)
add_definitions(-D_USE_MATH_DEFINES)
endif(WIN32)

find_library(ZLIB_LIBRARY NAMES zlib z)
find_path(ZLIB_INCLUDE_DIR NAMES zlib.h)

find_library(LIBGD_LIBRARY NAMES gd libgd libgd_static)
find_path(LIBGD_INCLUDE_DIR NAMES gd.h)

if(WIN32)
	include_directories(../wingetopt/src/)
	find_path(SYSTEM_INCLUDE_DIR dirent.h)
	include_directories(${SYSTEM_INCLUDE_DIR})

	add_library (wingetopt STATIC ../wingetopt/src/getopt.c)
endif()

set(wingetopt_lib "")
if(WIN32)
set(wingetopt_lib wingetopt)
endif()

set(leveldb_lib "")
if(USE_LEVELDB)
	set(leveldb_lib ${LEVELDB_LIBRARY})
	if(WIN32)
		# the leveld database driver requires PathFileExistsW which is in the Shlwapi library
		set(leveldb_lib ${LEVELDB_LIBRARY} "Shlwapi.lib")
	endif(WIN32)
endif()

set(postgresql_lib "")
if(USE_POSTGRESQL)
	set(postgresql_lib ${POSTGRESQL_LIBRARY})
	if(WIN32)
		# the libpq database driver requires htonl which is in the Ws2_32 library
		set(postgresql_lib ${POSTGRESQL_LIBRARY} "Ws2_32.lib")
	endif(WIN32)
endif()


#link_directories (../wingetopt)
# Fügen Sie der ausführbaren Datei für dieses Projekt eine Quelle hinzu.
add_executable (Minetestmapper ${sources})
set_target_properties(Minetestmapper PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(Minetestmapper ${wingetopt_lib} ${LIBGD_LIBRARY} ${ZLIB_LIBRARY} ${SQLITE3_LIBRARY} ${leveldb_lib} ${postgresql_lib})
# TODO: Fügen Sie bei Bedarf Tests und Installationsziele hinzu.


set (CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
include (InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_GENERATOR "ZIP")
include (CPack)
install (TARGETS Minetestmapper RUNTIME DESTINATION ".")

# Config might not set, so there might missing dll's after installation
install(DIRECTORY "${PROJECT_BINARY_DIR}/Minetestmapper/$<CONFIG>/" DESTINATION "." 
	FILES_MATCHING PATTERN "*.dll" PATTERN "/" EXCLUDE)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/colors/" DESTINATION "colors/" 
	FILES_MATCHING PATTERN "*.txt")

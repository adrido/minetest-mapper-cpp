# CMakeList.txt: CMake-Projekt für "Minetestmapper". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)

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
	Mapper.cpp
	Mapper.h
	main.cpp
	CharEncodingConverter.cpp
	CharEncodingConverter.h
	CharEncodingConverterIConv.cpp
	CharEncodingConverterIConv.h
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
	parg.h
	parg.c
	util.h
)
if(WIN32)
	list(APPEND sources Minetestmapper.rc)
endif(WIN32)

add_executable(Minetestmapper ${sources})

# Platform dependant stuff
if(WIN32)
	target_compile_definitions(Minetestmapper PRIVATE _USE_MATH_DEFINES)
elseif(UNIX)

endif()

# Required Libraries
###############################################################################
find_library(SQLITE3_LIBRARY sqlite3)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

find_package(ZLIB REQUIRED)

find_library(LIBGD_LIBRARY NAMES gd libgd libgd_static)
find_path(LIBGD_INCLUDE_DIR NAMES gd.h)

target_link_libraries(Minetestmapper ${LIBGD_LIBRARY} ZLIB::ZLIB ${SQLITE3_LIBRARY})
target_include_directories(Minetestmapper PRIVATE ${LIBGD_INCLUDE_DIR})

if(UNIX)
	find_package(PNG REQUIRED)
	find_package(Threads REQUIRED)

	target_link_libraries(Minetestmapper PNG::PNG Threads::Threads ${CMAKE_DL_LIBS})

	target_link_libraries(Minetestmapper $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
endif()

# Optional Libraries
###############################################################################
if(USE_LEVELDB)
	target_link_libraries(Minetestmapper ${LEVELDB_LIBRARY})
	if(WIN32)
		# the leveld database driver requires PathFileExistsW which is in the Shlwapi library
		target_link_libraries(Minetestmapper "Shlwapi.lib")
	endif(WIN32)
endif()

if(USE_POSTGRESQL)
	target_link_libraries(Minetestmapper ${POSTGRESQL_LIBRARY})
	if(WIN32)
		# the libpq database driver requires htonl which is in the Ws2_32 library
		target_link_libraries(Minetestmapper "Ws2_32.lib")
	endif(WIN32)
endif()

if(USE_REDIS)
	target_link_libraries(Minetestmapper ${REDIS_LIBRARY})
endif()

# Installation
###############################################################################


set (CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
include (InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_GENERATOR "ZIP")
include(CPack)
install(TARGETS Minetestmapper RUNTIME DESTINATION ".")

if(WIN32)
	#copy required dlls to install dir
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release/
			DESTINATION . 
			CONFIGURATIONS Release
			FILES_MATCHING PATTERN "*.dll")
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/
			DESTINATION .
			CONFIGURATIONS Debug
			FILES_MATCHING PATTERN "*.dll")
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/
			DESTINATION .
			CONFIGURATIONS RelWithDebInfo
			FILES_MATCHING PATTERN "*.dll")
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/
			DESTINATION .
			CONFIGURATIONS RelWithDebInfo
			FILES_MATCHING PATTERN "*.dll")
endif(WIN32)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/colors/" DESTINATION "colors/" 
	FILES_MATCHING PATTERN "*.txt")

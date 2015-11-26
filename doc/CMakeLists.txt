project(minetestmapper CXX)
cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0003 NEW)

if(NOT DEFINED USE_RST2HTML)
	set(USE_RST2HTML -1)
endif(NOT DEFINED USE_RST2HTML)

SET(REQUIRE_HTML_DOCUMENTATION_DOC "Convert rst documents to html or fail")
SET(DISABLE_HTML_DOCUMENTATION_DOC "Don't convert rst documents to html")
OPTION(REQUIRE_HTML_DOCUMENTATION ${REQUIRE_HTML_DOCUMENTATION_DOC} False)
OPTION(DISABLE_HTML_DOCUMENTATION ${DISABLE_HTML_DOCUMENTATION_DOC} False)

# Find rst to html converter
if(REQUIRE_HTML_DOCUMENTATION AND DISABLE_HTML_DOCUMENTATION)
	message(WARNING "REQUIRE_HTML_DOCUMENTATION and DISABLE_HTML_DOCUMENTATION are both enabled. Reinitializing rst to html conversion.")
	set(REQUIRE_HTML_DOCUMENTATION False CACHE BOOL ${REQUIRE_HTML_DOCUMENTATION_DOC} FORCE)
	set(DISABLE_HTML_DOCUMENTATION False CACHE BOOL ${DISABLE_HTML_DOCUMENTATION_DOC} FORCE)
	set(USE_RST2HTML -1)
endif(REQUIRE_HTML_DOCUMENTATION AND DISABLE_HTML_DOCUMENTATION)

find_package(Docutils)
if(DOCUTILS_FOUND)
	message(STATUS "Python docutils found (rst2html: ${RST2HTML_EXECUTABLE})")
else(DOCUTILS_FOUND)
	message(STATUS "Python docutils not found")
endif(DOCUTILS_FOUND)

if(REQUIRE_HTML_DOCUMENTATION)
	set(USE_RST2HTML 1)
	message(STATUS "Conversion of documentation to html enabled as requested")
elseif(DISABLE_HTML_DOCUMENTATION)
	set(USE_RST2HTML 0)
	message(STATUS "Conversion of documentation to html disabled as requested")
elseif(USE_RST2HTML EQUAL -1)
	# First time, and no preference specified: choose depending
	# on availability of rst2html
	if (DOCUTILS_FOUND)
		set(USE_RST2HTML 1)
		message(STATUS "Conversion of documentation to html enabled (because python docutils found)")
	else (DOCUTILS_FOUND)
		set(USE_RST2HTML 0)
		message(STATUS "Conversion of documentation to html disabled (because python docutils not found)")
	endif (DOCUTILS_FOUND)
elseif(USE_RST2HTML EQUAL 0)
	message(STATUS "Conversion of documentation to html disabled (because disabled previously)")
elseif(USE_RST2HTML EQUAL 1)
	message(STATUS "Conversion of documentation to html enabled (because enabled previously)")
endif(REQUIRE_HTML_DOCUMENTATION)

if(USE_RST2HTML AND NOT DOCUTILS_FOUND)
	message(SEND_ERROR "Conversion of documentation to html is enabled, but python docutils was not found.")
endif(USE_RST2HTML AND NOT DOCUTILS_FOUND)

# Save USE_RST2HTML for next invocation
set(USE_RST2HTML ${USE_RST2HTML} CACHE INTERNAL "Internal use - do not modify")

file(GLOB DOC_RST_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.rst)
string(REPLACE ".rst" ".html" DOC_HTML_FILES "${DOC_RST_FILES}")

if(USE_RST2HTML)
	add_custom_target(htmldoc ALL DEPENDS ${DOC_HTML_FILES})
elseif(NOT USE_RST2HTML AND DOCUTILS_FOUND)
	add_custom_target(htmldoc DEPENDS ${DOC_HTML_FILES})
endif(USE_RST2HTML)
RST2HTML(${DOC_RST_FILES})


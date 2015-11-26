# - Find python docutils
#
#  This module defines the following variables:
#
#   DOCUTILS_FOUND      - system has docutils
#
#   RST2HTML_EXECUTABLE - the rst2html executable

#=============================================================================
# Copyright 2010 Sascha Peilicke <sasch.pe@gmx.de
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_program(RST2HTML_EXECUTABLE NAMES rst2html.py rst2html
             DOC "The Python Docutils reStructuredText HTML converter")

# handle the QUIETLY and REQUIRED arguments and set DOCUTILS_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Docutils DEFAULT_MSG RST2HTML_EXECUTABLE)

if(RST2HTML_EXECUTABLE)
    macro(RST2HTML)
        foreach(it ${ARGN})
            get_filename_component(basename ${it} NAME_WE)
            set(infile ${CMAKE_CURRENT_SOURCE_DIR}/${it})
            set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${basename}.html)

            add_custom_command(OUTPUT ${outfile} COMMAND ${RST2HTML_EXECUTABLE}
                               ARGS ${infile} ${outfile} DEPENDS ${infile})
            add_custom_target(RstHTMLDocuments_${it} ALL DEPENDS ${outfile})
        endforeach()
    endmacro()
endif()

mark_as_advanced(RST2HTML_EXECUTABLE)

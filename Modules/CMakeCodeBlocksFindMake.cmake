
#=============================================================================
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_program(CMAKE_MAKE_PROGRAM NAMES codeblocks_con codeblocks
             DOC "The executable of the Code::Blocks IDE")
mark_as_advanced(CMAKE_MAKE_PROGRAM)

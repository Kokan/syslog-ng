#############################################################################
# Copyright (c) 2017 Balabit
# Copyright (c) 2017 Kokan
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
#############################################################################

# - Try to find libmongoc-1.0
# Once done this will define
#  MONGOC_FOUND - System has libmongoc-1.0
#  MONGOC_INCLUDE_DIR - The libmongoc-1.0 include directories
#  MONGOC_LIBRARY - The libraries needed to use libmongoc-1.0
#  MONGOC_DEFINITIONS - Compiler switches required for using libmongoc-1.0

find_package(PkgConfig)
pkg_check_modules(PC_MONGOC QUIET libmongoc-1.0)
set(MONGOC_DEFINITIONS ${PC_MONGOC_CFLAGS_OTHER})

find_path(LIBMONGOC_INCLUDE_DIR mongoc.h
          HINTS ${PC_MONGOC_INCLUDEDIR} ${PC_MONGOC_INCLUDE_DIRS}
          PATH_SUFFIXES libmongoc-1.0 )

find_library(LIBMONGOC_LIBRARY NAMES mongoc mongoc-1.0
             HINTS ${PC_MONGOC_LIBDIR} ${PC_MONGOC_LIBRARY_DIRS} )

find_path(BSON_INCLUDE_DIR bson.h
          HINTS ${PC_MONGOC_INCLUDEDIR} ${PC_MONGOC_INCLUDE_DIRS})

find_library(BSON_LIBRARY NAMES bson bson-1.0
             HINTS ${PC_MONGOC_LIBDIR} ${PC_MONGOC_LIBRARY_DIRS} )

set(MONGOC_LIBRARY ${LIBMONGOC_LIBRARY} ${BSON_LIBRARY} CACHE STRING "MongoC library path")
set(MONGOC_INCLUDE_DIR ${LIBMONGOC_INCLUDE_DIR} ${BSON_INCLUDE_DIR} CACHE STRING "MongoC include path")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MONGOC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(MONGOC DEFAULT_MSG MONGOC_LIBRARY MONGOC_INCLUDE_DIR)

mark_as_advanced(MONGOC_INCLUDE_DIR MONGOC_LIBRARY
                 LIBMONGOC_INCLUDE_DIR LIBMONGOC_INCLUDE_DIR 
                 BSON_INCLUDE_DIR BSON_LIBRARY)


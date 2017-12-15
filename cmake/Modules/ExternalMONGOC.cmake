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

set(LIB_NAME "MONGOC")

if (EXISTS ${PROJECT_SOURCE_DIR}/modules/afmongodb/mongo-c-driver/src/mongoc/mongoc.h)

# external cmake version missing libbson
#    ExternalProject_Add(
#        ${LIB_NAME}
#        PREFIX            ${CMAKE_CURRENT_BINARY_DIR}
#        INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}
#        SOURCE_DIR        ${PROJECT_SOURCE_DIR}/afmongodb/mongo-c-driver/
#        DOWNLOAD_COMMAND  echo
#        BUILD_COMMAND     make
#        INSTALL_COMMAND   make install
#        CONFIGURE_COMMAND cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/modules/afmongodb/mongo-c-driver
#    )
    ExternalProject_Add(
        ${LIB_NAME}
        PREFIX            ${CMAKE_CURRENT_BINARY_DIR}
        INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}
        SOURCE_DIR        ${PROJECT_SOURCE_DIR}/afmongodb/mongo-c-driver/
        DOWNLOAD_COMMAND  echo
        BUILD_COMMAND     make
        INSTALL_COMMAND   make install
        CONFIGURE_COMMAND autoreconf -i ${PROJECT_SOURCE_DIR}/modules/afmongodb/mongo-c-driver/ && 
                          ${PROJECT_SOURCE_DIR}/modules/afmongodb/mongo-c-driver/configure --enable-tests=no --with-libbson=bundled --prefix=${PROJECT_BINARY_DIR}
        STEP_TARGETS update patch configure
    )

    set(${LIB_NAME}_INTERNAL TRUE)
    set(${LIB_NAME}_INTERNAL_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include/libmongoc-1.0/;${PROJECT_BINARY_DIR}/include/libbson-1.0/")
    set(${LIB_NAME}_INTERNAL_LIBRARY "${PROJECT_BINARY_DIR}/lib/libmongoc-1.0.so")
    install(DIRECTORY ${PROJECT_BINARY_DIR}/lib/ DESTINATION lib USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "libmongoc-1.0.so*")

else()
  set(${LIB_NAME}_INTERNAL FALSE)
endif()




external_or_find_package(MONGOC)

if (MONGOC_FOUND)
  option(ENABLE_AFMONGODB "Enable afmongodb module" ON)
else()
  option(ENABLE_AFMONGODB "Enable afmongodb module" OFF)
endif()

if (ENABLE_AFMONGODB)

set(AFMONGODB_HEADERS
    "afmongodb-parser.h"
    "afmongodb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/afmongodb-grammar.h"
)

set(AFMONGODB_SOURCES
    "afmongodb-parser.c"
    "afmongodb.c"
    "${CMAKE_CURRENT_BINARY_DIR}/afmongodb-grammar.c"
)

generate_y_from_ym(modules/afmongodb/afmongodb-grammar)

bison_target(MongoDBGrammar
    ${CMAKE_CURRENT_BINARY_DIR}/afmongodb-grammar.y
    ${CMAKE_CURRENT_BINARY_DIR}/afmongodb-grammar.c
    COMPILE_FLAGS ${BISON_FLAGS})

include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${MONGOC_INCLUDE_DIR})
add_library(afmongodb SHARED ${AFMONGODB_SOURCES})
target_link_libraries(afmongodb PUBLIC ${MONGOC_LIBRARY})
target_link_libraries(afmongodb PRIVATE syslog-ng)

if (MONGOC_INTERNAL)
   add_dependencies(afmongodb MONGOC)
endif()

install(TARGETS afmongodb LIBRARY DESTINATION lib/syslog-ng/ COMPONENT afmongodb)

endif()

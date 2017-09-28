option(ENABLE_HEARTBEAT "Enable heartbeat" ON)

if (ENABLE_HEARTBEAT)

set(HEARTBEAT_HEADERS
    "heartbeat-parser.h"
    "${CMAKE_CURRENT_BINARY_DIR}/heartbeat-grammar.h"
)

set(HEARTBEAT_SOURCES
    "heartbeat-plugin.c"
    "heartbeat-parser.c"
    "${CMAKE_CURRENT_BINARY_DIR}/heartbeat-grammar.c"
)

generate_y_from_ym(modules/heartbeat/heartbeat-grammar)

bison_target(heartbeatGrammar
    ${CMAKE_CURRENT_BINARY_DIR}/heartbeat-grammar.y
    ${CMAKE_CURRENT_BINARY_DIR}/heartbeat-grammar.c
COMPILE_FLAGS ${BISON_FLAGS})
  include_directories (${CMAKE_CURRENT_BINARY_DIR})
  include_directories (${CMAKE_CURRENT_SOURCE_DIR})
  add_library(heartbeat MODULE ${heartbeat_SOURCES})
  target_link_libraries(heartbeat PRIVATE syslog-ng)

  install(TARGETS heartbeat
  LIBRARY DESTINATION lib/syslog-ng/
  COMPONENT heartbeat)
endif()

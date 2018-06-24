
add_executable(pdbtool pdbtool.c)
target_link_libraries(pdbtool patterndb eventlog)
install(TARGETS pdbtool RUNTIME DESTINATION bin)

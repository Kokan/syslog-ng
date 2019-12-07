add_unit_test(CRITERION LIBTEST TARGET test_logmsg_serialize DEPENDS syslogformat)
add_unit_test(CRITERION LIBTEST TARGET test_timestamp_serialize)
add_unit_test(CRITERION TARGET test_tags)
add_unit_test(CRITERION TARGET test_nvtable)
add_unit_test(CRITERION TARGET test_gsockaddr_serialize)
add_unit_test(CRITERION LIBTEST TARGET test_log_message)
add_unit_test(CRITERION TARGET test_logmsg_ack)
add_unit_test(CRITERION TARGET test_nvhandle_desc_array)

if (FUZZER)

add_executable(test_fuzzing_logmsg_serialize test_fuzzing_logmsg_serialize.c)
target_link_libraries(test_fuzzing_logmsg_serialize PRIVATE syslog-ng)
set_target_properties(test_fuzzing_logmsg_serialize PROPERTIES COMPILE_FLAGS "-O1 -fsanitize=\"${SANITIZER},fuzzer\" -fno-omit-frame-pointer")
set_target_properties(test_fuzzing_logmsg_serialize PROPERTIES LINK_FLAGS "-O1 -fsanitize=\"${SANITIZER},fuzzer\" -fno-omit-frame-pointer")

endif()

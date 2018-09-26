modules_afamqp_tests_TESTS		= \
	modules/afamqp/tests/test_afamqp

check_PROGRAMS				+= ${modules_afamqp_tests_TESTS}

modules_afamqp_tests_test_afamqp_CFLAGS	= $(TEST_CFLAGS) -I$(top_srcdir)/modules/afamqp
modules_afamqp_tests_test_afamqp_LDADD	= $(TEST_LDADD)
modules_afamqp_tests_test_afamqp_LDFLAGS	= \
	-dlpreopen $(top_builddir)/modules/afamqp/libafamqp.la
modules_afamqp_tests_test_afamqp_DEPENDENCIES = $(top_builddir)/modules/afamqp/libafamqp.la

EXTRA_DIST += modules/afamqp/tests/CMakeLists.txt

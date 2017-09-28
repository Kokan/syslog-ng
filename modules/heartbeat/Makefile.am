module_LTLIBRARIES      += modules/heartbeat/libheartbeat.la
modules_heartbeat_libheartbeat_la_SOURCES = \
  modules/heartbeat/heartbeat-grammar.y       \
  modules/heartbeat/heartbeat-parser.c        \
  modules/heartbeat/heartbeat-parser.h        \
  modules/heartbeat/heartbeat-plugin.c        \
  modules/heartbeat/heartbeat-source-driver.c \
  modules/heartbeat/heartbeat-source.c        \
  modules/heartbeat/heartbeat-option.c

BUILT_SOURCES       +=      \
  modules/heartbeat/heartbeat-grammar.y       \
  modules/heartbeat/heartbeat-grammar.c       \
  modules/heartbeat/heartbeat-grammar.h

EXTRA_DIST        +=      \
  modules/heartbeat/heartbeat-grammar.ym

modules_heartbeat_libheartbeat_la_CPPFLAGS  =     \
  $(AM_CPPFLAGS)            \
  -I$(top_srcdir)/modules/heartbeat        \
  -I$(top_builddir)/modules/heartbeat
modules_heartbeat_libheartbeat_la_LIBADD  = $(MODULE_DEPS_LIBS)
modules_heartbeat_libheartbeat_la_LDFLAGS = $(MODULE_LDFLAGS)
modules_heartbeat_libheartbeat_la_DEPENDENCIES= $(MODULE_DEPS_LIBS)

modules/heartbeat modules/heartbeat/ mod-heartbeat mod-file: modules/heartbeat/libheartbeat.la
.PHONY: modules/heartbeat/ mod-heartbeat mod-file



#include <stdint.h>
#include <stddef.h>

#include "apphook.h"
#include "logmsg/logmsg-serialize.h"
#include "logmsg/logmsg.h"
#include "scratch-buffers.h"
#include <iv.h>

static void
_reset_log_msg_registry(void)
{
  log_msg_registry_deinit();
  log_msg_registry_init();
}

void
msg_send_formatted_message(int prio, const char *msg)
{
}


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size <= 1) return 0;

  app_startup();

  GString serialized = {0};
  serialized.allocated_len = 0;
  serialized.len = size;
  serialized.str = (gchar *)data;
  LogMessage *msg = log_msg_new_empty();

  SerializeArchive *sa = serialize_string_archive_new(&serialized);
  _reset_log_msg_registry();

  log_msg_deserialize(msg, sa);

  log_msg_unref(msg);
  // Despite the SerializeArchive is created via new, the serialized data came from the fuzzer as input.
  // As far as the libFuzzer goes, this function do not own the data, just a view. Thus it should not free.
  //serialize_archive_free(sa);

  scratch_buffers_explicit_gc();

  app_shutdown();
  iv_deinit();
  return 0;
}




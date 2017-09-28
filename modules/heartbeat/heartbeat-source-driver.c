#include "heartbeat-source-driver.h"

#include "heartbeat-option.h"
#include "heartbeat-source.h"

#include "logsource.h"

typedef struct _HeartBeatSourceDriver
{
  LogSrcDriver      super;
  LogSource        *source;
  LogSourceOptions *options;
} HeartBeatSourceDriver;

static void
_heartbeat_sd_free(LogPipe *s)
{
  HeartBeatSourceDriver *self = (HeartBeatSourceDriver *)s;

  heartbeat_option_free(self->options);
  log_src_driver_free(s);
}

static gboolean
_heartbeat_sd_deinit(LogPipe *s)
{
  HeartBeatSourceDriver *self = (HeartBeatSourceDriver *)s;

  if (self->source)
    {
      log_pipe_deinit(&self->source->super);
      log_pipe_unref(&self->source->super);
      self->source = NULL;
    }

  if (!log_src_driver_deinit_method(s))
    return FALSE;


  return TRUE;
}

static gboolean
_heartbeat_sd_init(LogPipe *s)
{
  HeartBeatSourceDriver *self = (HeartBeatSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  if (!log_src_driver_init_method(s))
    return FALSE;

  heartbeat_option_init(self->options, cfg);
  self->source = heartbeat_source_new(&self->super, self->options);
  log_pipe_append(&self->source->super, s);

  if (!log_pipe_init(&self->source->super))
    {
      log_pipe_unref(&self->source->super);
      self->source = NULL;
      return FALSE;
    }

  return TRUE;
}

LogDriver *
heartbeat_sd_new(GlobalConfig *cfg)
{
  HeartBeatSourceDriver *self = g_new0(HeartBeatSourceDriver, 1);

  log_src_driver_init_instance(&self->super, cfg);

  self->super.super.super.init    = _heartbeat_sd_init;
  self->super.super.super.deinit  = _heartbeat_sd_deinit;
  self->super.super.super.free_fn = _heartbeat_sd_free;

  self->options = heartbeat_option_new();

  return &self->super.super;
}

void
heartbeat_sd_set_frequency(LogDriver *s, gfloat freq)
{
  HeartBeatSourceDriver *self = (HeartBeatSourceDriver *)s;

  heartbeat_option_set_freq(self->options, freq);
}

LogSourceOptions *heartbeat_sd_get_option(LogDriver *s)
{
  HeartBeatSourceDriver *self = (HeartBeatSourceDriver *)s;

  return self->options;
}


#include "heartbeat-option.h"

typedef struct _HeartBeatOptions
{
  LogSourceOptions super;
  gfloat freq;
  LogTemplate *template;
  LogTemplateOptions template_options;
} HeartBeatOptions;

LogSourceOptions *heartbeat_option_new()
{
  HeartBeatOptions *self = g_new0(HeartBeatOptions, 1);
  log_source_options_defaults(&self->super);

  return &self->super;
}

void heartbeat_option_free(LogSourceOptions *self)
{
  log_source_options_destroy(self);
  g_free(self);
}

void heartbeat_option_init(LogSourceOptions *self, GlobalConfig *cfg)
{
  log_source_options_init(self, cfg, "dummy-group-name");
}

void heartbeat_option_set_freq(LogSourceOptions *s, gfloat freq)
{
  HeartBeatOptions *self = (HeartBeatOptions *)s;
  msg_verbose("set_freq");

  self->freq = (int)(1000*freq);
}

int heartbeat_option_get_freq(LogSourceOptions *s)
{
  HeartBeatOptions *self = (HeartBeatOptions *)s;

  return self->freq;
}

LogTemplate *heartbeat_option_get_template(LogSourceOptions *s)
{
  HeartBeatOptions *self = (HeartBeatOptions *)s;

  return self->template;
}

void heartbeat_option_set_template(LogSourceOptions *s,LogTemplate *template)
{
  HeartBeatOptions *self = (HeartBeatOptions *)s;

  log_template_unref(self->template);

  self->template = template;
}

LogTemplateOptions *heartbeat_option_get_template_options(LogSourceOptions *s)
{
  HeartBeatOptions *self = (HeartBeatOptions *)s;

  return &self->template_options;
}


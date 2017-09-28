#include "heartbeat-source.h"
#include "heartbeat-option.h"

#include <iv.h>

typedef struct _HeartBeatSource
{
  LogSource         super;
  LogSourceOptions *options;
  struct iv_timer    beat;
  gint               seq_num;
} HeartBeatSource;

void _send_heartbeat_message(void *p)
{
  HeartBeatSource *self = (HeartBeatSource *)p;

  if (log_source_free_to_send(&self->super))
    {
      LogMessage *msg;

      msg = log_msg_new_empty();
      log_msg_set_value(msg, LM_V_PROGRAM, "syslog-ng", 9);
      //log_msg_set_value(msg, LM_V_PID, buf, -1);
      msg->flags |= LF_LOCAL;
      msg->flags |= LF_SIMPLE_HOSTNAME;

      ++self->seq_num;

      GString *formatted_message = g_string_new("HB");
      LogTemplate *template = heartbeat_option_get_template(self->options);
      if (template)
        {
          LogTemplateOptions *toption = heartbeat_option_get_template_options(self->options);

          log_template_format(template, msg, toption, LTZ_LOCAL, self->seq_num, NULL,
                              formatted_message);
        }
      log_msg_set_value(msg, LM_V_MESSAGE, formatted_message->str, -1);
      g_string_free(formatted_message, TRUE);

      log_source_post(&self->super, msg);
    }
}

void iv_timer_set_timeout(struct iv_timer *timer, int diff)
{
  if (iv_timer_registered(timer))
    iv_timer_unregister(timer);

  iv_validate_now();
  timer->expires = iv_now;
  timespec_add_msec(&timer->expires, diff);

  iv_timer_register(timer);
}

static void
_heartbeat_schedule_next_beat(HeartBeatSource *self)
{
  gfloat shift = heartbeat_option_get_freq(self->options);

  iv_timer_set_timeout(&self->beat,shift);
}

static void
_heartbeat_heart_beat_handle(void *p)
{
  HeartBeatSource *self = (HeartBeatSource *)p;

  _send_heartbeat_message(p);
  _heartbeat_schedule_next_beat(self);
}

static gboolean
_heartbeat_source_init(LogPipe *s)
{
  HeartBeatSource *self = (HeartBeatSource *)s;

  if (!log_source_init(s))
    return FALSE;

  IV_TIMER_INIT(&self->beat);
  self->beat.cookie = self;
  self->beat.handler = _heartbeat_heart_beat_handle;

  _heartbeat_schedule_next_beat(self);

  self->seq_num = 0;

  return TRUE;
}


static gboolean
_heartbeat_source_deinit(LogPipe *s)
{
  HeartBeatSource *self = (HeartBeatSource *)s;

  if (iv_timer_registered(&self->beat))
    iv_timer_unregister(&self->beat);

  return TRUE;
}

static void
_heartbeat_source_free(LogPipe *s)
{
  HeartBeatSource *self = (HeartBeatSource *)s;

  log_source_free(&self->super.super);
}

LogSource *heartbeat_source_new(LogSrcDriver *owner, LogSourceOptions *options)
{
  HeartBeatSource *self = g_new0(HeartBeatSource, 1);

  log_source_init_instance(&self->super, owner->super.super.cfg);
  log_source_set_options(&self->super, options, owner->super.id, NULL, FALSE, FALSE, owner->super.super.expr_node);

  self->options = options;
  self->super.super.init    = _heartbeat_source_init;
  self->super.super.deinit  = _heartbeat_source_deinit;
  self->super.super.free_fn = _heartbeat_source_free;

  return &self->super;
}


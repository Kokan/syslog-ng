#ifndef HEARTBEAT_OPTION_H_INCLUDED
#define HEARTBEAT_OPTION_H_INCLUDED

#include "cfg.h"
#include "logsource.h"

struct HeartBeatOptions;

LogSourceOptions *heartbeat_option_new();
void heartbeat_option_free(LogSourceOptions *);
void heartbeat_option_init(LogSourceOptions *, GlobalConfig*);
void heartbeat_option_set_freq(LogSourceOptions*,gfloat);
int heartbeat_option_get_freq(LogSourceOptions*);
LogTemplate* heartbeat_option_get_template(LogSourceOptions*);
void heartbeat_option_set_template(LogSourceOptions*,LogTemplate*);
LogTemplateOptions* heartbeat_option_get_template_options(LogSourceOptions*);

#endif //HEARTBEAT_OPTION_H_INCLUDED

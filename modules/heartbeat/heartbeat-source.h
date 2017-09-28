#ifndef HEARTBEAT_SOURCE_H_INCLUDED
#define HEARTBEAT_SOURCE_H_INCLUDED

#include "logsource.h"
#include "driver.h"

struct HeartBeatSource;

LogSource* heartbeat_source_new(LogSrcDriver*, LogSourceOptions*);
void heartbeat_set_template(LogSrcDriver*, LogTemplate*);

#endif //HEARTBEAT_SOURCE_H_INCLUDED

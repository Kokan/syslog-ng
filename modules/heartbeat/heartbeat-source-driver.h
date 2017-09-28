
#ifndef HEARTBEAT_SOURCE_DRIVER_H_INCLUDED
#define HEARTBEAT_SOURCE_DRIVER_H_INCLUDED

#include "driver.h"
#include "logsource.h"

struct HeartBeatSourceDriver;

LogDriver *heartbeat_sd_new(GlobalConfig*);

void heartbeat_sd_set_frequency(LogDriver*, gfloat);
LogSourceOptions* heartbeat_sd_get_option(LogDriver*);

#endif //HEARTBEAT_SOURCE_DRIVER_H_INCLUDED


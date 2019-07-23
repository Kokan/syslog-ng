/*
 * Copyright (c) 2014 Pierre-Yves Ritschard <pyr@spootnik.org>
 * Copyright (c) 2013-2019 Balabit
 * Copyright (c) 2019 Balazs Scheidler
 * Copyright (c) 2019 Kokan <kokaipeter@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef RDKAFKA_COMMON_H_INCLUDED
#define RDKAFKA_COMMON_H_INCLUDED

#include <librdkafka/rdkafka.h>
#include "kafka-props.h"

void _conf_set_prop(rd_kafka_conf_t *conf, const gchar *name, const gchar *value);
gboolean rd_kafka_conf_apply_properties(rd_kafka_conf_t *conf, KafkaProperties *props);
rd_kafka_conf_t *_construct_kafka_conf(KafkaProperties *properties, const gchar *bootstrap_servers);
rd_kafka_topic_t *_construct_kafka_topic(rd_kafka_t *kafka, const gchar *topic_name);



#endif

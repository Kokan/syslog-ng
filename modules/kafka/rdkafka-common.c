/*
 * Copyright (c) 2013 Tihamer Petrovics <tihameri@gmail.com>
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

#include "rdkafka-common.h"
#include "messages.h"

#include <stdlib.h>

void
_kafka_log_callback(const rd_kafka_t *rkt, int level, const char *fac, const char *msg)
{
  gchar *buf = g_strdup_printf("librdkafka: %s(%d): %s", fac, level, msg);
  msg_event_send(msg_event_create(level, buf, NULL));
  g_free(buf);
}


void
_conf_set_prop(rd_kafka_conf_t *conf, const gchar *name, const gchar *value)
{
  gchar errbuf[1024];

  msg_debug("kafka: setting librdkafka config property",
            evt_tag_str("name", name),
            evt_tag_str("value", value));
  if (rd_kafka_conf_set(conf, name, value, errbuf, sizeof(errbuf)) < 0)
    {
      msg_error("kafka: error setting librdkafka config property",
                evt_tag_str("name", name),
                evt_tag_str("value", value),
                evt_tag_str("error", errbuf));
    }
}

gboolean
rd_kafka_conf_apply_properties(rd_kafka_conf_t *conf, KafkaProperties *props)
{
  GList *ll;

  for (ll = props; ll != NULL; ll = g_list_next(ll))
    {
      KafkaProperty *kp = ll->data;
      _conf_set_prop(conf, kp->name, kp->value);
    }
  return TRUE;
}

rd_kafka_conf_t *
_construct_kafka_conf(KafkaProperties *properties, const gchar *bootstrap_servers)
{
  rd_kafka_conf_t *conf;

  conf = rd_kafka_conf_new();

  _conf_set_prop(conf, "metadata.broker.list", bootstrap_servers);

  rd_kafka_conf_apply_properties(conf, properties);
  rd_kafka_conf_set_log_cb(conf, _kafka_log_callback);


  return conf;
}

rd_kafka_topic_t *
_construct_kafka_topic(rd_kafka_t *kafka, const gchar *topic_name)
{
  g_assert(kafka != NULL);

  return rd_kafka_topic_new(kafka, topic_name, NULL);
}


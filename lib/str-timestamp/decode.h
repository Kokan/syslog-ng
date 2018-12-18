/*
 * Copyright (c) 2002-2018 Balabit
 * Copyright (c) 1998-2018 Balázs Scheidler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef STR_TIMESTAMP_DECODE_H_INCLUDED
#define STR_TIMESTAMP_DECODE_H_INCLUDED

#include "syslog-ng.h"
#include "logstamp.h"

gboolean scan_iso_timestamp(const gchar **buf, gint *left, struct tm *tm);
gboolean scan_pix_timestamp(const gchar **buf, gint *left, struct tm *tm);
gboolean scan_linksys_timestamp(const gchar **buf, gint *left, struct tm *tm);
gboolean scan_bsd_timestamp(const gchar **buf, gint *left, struct tm *tm);

gboolean scan_rfc3164_timestamp(const guchar **data, gint *length, LogStamp *stamp,
                                gboolean ignore_result, glong default_timezone);
gboolean scan_rfc5424_timestamp(const guchar **data, gint *length, LogStamp *stamp,
                                gboolean ignore_result, glong default_timezone);

#endif

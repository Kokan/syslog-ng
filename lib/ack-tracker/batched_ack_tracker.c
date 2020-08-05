/*
 * Copyright (c) 2020 Balabit
 * Copyright (c) 2020 Kokan
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

#include "batched_ack_tracker.h"
#include "bookmark.h"
#include "syslog-ng.h"

typedef struct _BatchedAckRecord
{
  AckRecord super;
  Bookmark bookmark;
} BatchedAckRecord;

typedef struct BatchedAckTracker
{
  AckTracker super;
  GMutex mutex;
  GList *acked_records;
  BatchedAckRecord *pending_ack_record;
  gint min_batch_size;
} BatchedAckTracker;

void
batched_ack_tracker_lock(AckTracker *s)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  g_mutex_lock(&self->mutex);
}

void
batched_ack_tracker_unlock(AckTracker *s)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  g_mutex_unlock(&self->mutex);
}

static inline void
batched_ack_record_destroy(BatchedAckRecord *self)
{
  bookmark_destroy(&self->bookmark);
}

static Bookmark *
batched_ack_tracker_request_bookmark(AckTracker *s)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  if (!self->pending_ack_record)
    {
      batched_ack_tracker_lock(s);
      {
        self->pending_ack_record = g_new(BatchedAckRecord, 1);
      }
      batched_ack_tracker_unlock(s);
    }

  if (self->pending_ack_record)
    {
      self->pending_ack_record->bookmark.persist_state = s->source->super.cfg->state;

      self->pending_ack_record->super.tracker = (AckTracker *)self;

      return &(self->pending_ack_record->bookmark);
    }

  return NULL;
}

static void
batched_ack_tracker_track_msg(AckTracker *s, LogMessage *msg)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;
  log_pipe_ref((LogPipe *)self->super.source);

  g_assert(self->pending_ack_record != NULL);

  batched_ack_tracker_lock(s);
  {
    msg->ack_record = (AckRecord *)self->pending_ack_record;
    self->pending_ack_record = NULL;
  }
  batched_ack_tracker_unlock(s);
}

static void
_ack_record_save_bookmark(gpointer p)
{
  BatchedAckRecord *self = (BatchedAckRecord *)p;

  Bookmark *bookmark = &(self->bookmark);
  bookmark_save( bookmark );

  bookmark_destroy( bookmark );

  g_free(self);
}

void
batched_ack_tracker_save_batched_acks(AckTracker *s)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  batched_ack_tracker_lock(s);
  {
    gint number_of_acked_msg = g_list_length(self->acked_records);
    if (number_of_acked_msg < self->min_batch_size)
      {
        batched_ack_tracker_unlock(s);
        return;
      }
    g_list_free_full(self->acked_records, _ack_record_save_bookmark);
    self->acked_records = NULL;
  }
  batched_ack_tracker_unlock(s);
}

static void
batched_ack_tracker_manage_msg_ack(AckTracker *s, LogMessage *msg, AckType ack_type)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  log_source_flow_control_adjust(self->super.source, 1);

  if (ack_type == AT_SUSPENDED)
    log_source_flow_control_suspend(self->super.source);

  if (ack_type != AT_ABORTED)
    {
      batched_ack_tracker_lock(s);
      {
        self->acked_records = g_list_append(self->acked_records, msg->ack_record);
      }
      batched_ack_tracker_unlock(s);
    }

  log_msg_unref(msg);
  log_pipe_unref((LogPipe *)self->super.source);
}

static void
_free_and_destroy_ack_record(gpointer p)
{
  BatchedAckRecord *self = (BatchedAckRecord *)p;

  batched_ack_record_destroy(self);
  g_free(self);
}

static void
batched_ack_tracker_free(AckTracker *s)
{
  BatchedAckTracker *self = (BatchedAckTracker *)s;

  if (self->pending_ack_record)
    _free_and_destroy_ack_record(self->pending_ack_record);

  g_list_free_full(self->acked_records, _free_and_destroy_ack_record);

  g_mutex_clear(&self->mutex);

  g_free(self);
}

AckTracker *
batched_ack_tracker_new(LogSource *source, gint min_batch_size, gint timeout)
{
  BatchedAckTracker *self = g_new0(BatchedAckTracker, 1);

  self->min_batch_size = min_batch_size;

  source->ack_tracker = (AckTracker *)self;
  self->super.source = source;

  g_mutex_init(&self->mutex);

  self->super.request_bookmark = batched_ack_tracker_request_bookmark;
  self->super.track_msg = batched_ack_tracker_track_msg;
  self->super.manage_msg_ack = batched_ack_tracker_manage_msg_ack;
  self->super.free_fn = batched_ack_tracker_free;

  return (AckTracker *)self;
}


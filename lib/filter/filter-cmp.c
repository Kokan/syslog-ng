/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2013 Balázs Scheidler
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

#include "filter/filter-cmp.h"
#include "filter/filter-expr-grammar.h"
#include "scratch-buffers.h"

#include <stdlib.h>
#include <string.h>

#define FCMP_EQ  0x0001
#define FCMP_LT  0x0002
#define FCMP_GT  0x0004
#define FCMP_NUM 0x0010

typedef struct _FilterCmp
{
  FilterExprNode super;
  LogTemplate *left, *right;
  gint cmp_op;
} FilterCmp;

static const gchar *
_format_template(LogTemplate *template, LogMessage **msgs, gint num_msg)
{
  if (log_template_is_trivial(template))
     return log_template_get_trivial_value(template, msgs[num_msg - 1], NULL);

  GString *buffer = scratch_buffers_alloc();

  log_template_format_with_context(template, msgs, num_msg, NULL, LTZ_LOCAL, 0, NULL, buffer);

  return buffer->str;
}

static gint
fop_compare(FilterCmp *self, const gchar *left, const gchar *right)
{
  gint cmp;
  if (self->cmp_op & FCMP_NUM)
    {
      gint l, r;

      l = atoi(left);
      r = atoi(right);
      if (l == r)
        cmp = 0;
      else if (l < r)
        cmp = -1;
      else
        cmp = 1;
    }
  else
    {
      cmp = strcmp(left, right);
    }

  return cmp;
}

static gboolean
fop_cmp_eval(FilterExprNode *s, LogMessage **msgs, gint num_msg)
{
  FilterCmp *self = (FilterCmp *) s;

  ScratchBuffersMarker marker;
  scratch_buffers_mark(&marker);

  const gchar *left_template  = _format_template(self->left, msgs, num_msg);
  const gchar *right_template = _format_template(self->right, msgs, num_msg);

  msg_trace("cmp() evaluation started",
            evt_tag_str("left", left_template),
            evt_tag_str("operator", self->super.type),
            evt_tag_str("right", right_template),
            evt_tag_printf("msg", "%p", msgs[num_msg - 1]));

  gboolean result = FALSE;

  gint cmp = fop_compare(self, left_template, right_template);

  if (cmp == 0)
    {
      result = self->cmp_op & FCMP_EQ;
    }
  else if (cmp < 0)
    {
      result = !!(self->cmp_op & FCMP_LT);
    }
  else
    {
      result = !!(self->cmp_op & FCMP_GT);
    }

  scratch_buffers_reclaim_marked(marker);
  return result ^ s->comp;
}

static void
fop_cmp_free(FilterExprNode *s)
{
  FilterCmp *self = (FilterCmp *) s;

  log_template_unref(self->left);
  log_template_unref(self->right);
}

static void
fop_map_grammar_token_to_cmp_op(FilterCmp *self, GlobalConfig *cfg, gint token)
{
  switch (token)
    {
    case KW_NUM_LT:
      self->cmp_op = FCMP_NUM;
    case KW_LT:
      self->cmp_op |= FCMP_LT;
      self->super.type = "<";
      break;

    case KW_NUM_LE:
      self->cmp_op = FCMP_NUM;
    case KW_LE:
      self->cmp_op |= FCMP_LT | FCMP_EQ;
      self->super.type = "<=";
      break;

    case KW_NUM_EQ:
      self->cmp_op = FCMP_NUM;
    case KW_EQ:
      self->cmp_op |= FCMP_EQ;
      self->super.type = "==";
      break;

    case KW_NUM_NE:
      self->cmp_op = FCMP_NUM;
    case KW_NE:
      self->cmp_op |= FCMP_LT | FCMP_GT;
      self->super.type = "!=";
      break;

    case KW_NUM_GE:
      self->cmp_op = FCMP_NUM;
    case KW_GE:
      self->cmp_op |= FCMP_GT | FCMP_EQ;
      self->super.type = ">=";
      break;

    case KW_NUM_GT:
      self->cmp_op = FCMP_NUM;
    case KW_GT:
      self->cmp_op |= FCMP_GT;
      self->super.type = ">";
      break;

    default:
      g_assert_not_reached();
    }

  if (self->cmp_op & FCMP_NUM && cfg_is_config_version_older(cfg, VERSION_VALUE_3_8))
    {
      msg_warning("WARNING: due to a bug in versions before " VERSION_3_8
                  "numeric comparison operators like '!=' in filter "
                  "expressions were evaluated as string operators. This is fixed in " VERSION_3_8 ". "
                  "As we are operating in compatibility mode, syslog-ng will exhibit the buggy "
                  "behaviour as previous versions until you bump the @version value in your "
                  "configuration file");
      self->cmp_op &= ~FCMP_NUM;
    }
}

FilterExprNode *
fop_cmp_new(LogTemplate *left, LogTemplate *right, gint token)
{
  FilterCmp *self = g_new0(FilterCmp, 1);

  filter_expr_node_init_instance(&self->super);

  fop_map_grammar_token_to_cmp_op(self, left->cfg, token);

  self->super.eval = fop_cmp_eval;
  self->super.free_fn = fop_cmp_free;
  self->left = left;
  self->right = right;

  return &self->super;
}

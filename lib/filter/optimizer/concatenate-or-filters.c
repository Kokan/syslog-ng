/*
 * Copyright (c) 2019 Balabit
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
#include "filter/optimizer/concatenate-or-filters.h"
#include "filter/filter-re.h"


static gboolean
_is_it_template(gchar *candidate)
{
  return (strchr(candidate, '$') != NULL);
}

static FilterExprNode *
_concatenate(FilterExprNode *current, FilterExprNode *parent, FilterExprNode *left, FilterExprNode *right)
{
  const gboolean is_it_template = _is_it_template(left->template);

  GString *pattern = g_string_new("");
  g_string_printf(pattern, "%s|%s", left->pattern, right->pattern);

  FilterExprNode *new_filter = filter_match_new();

  GError *error = NULL;
  g_assert(filter_re_compile_pattern(new_filter, pattern->str, &error));

  if (is_it_template)
    {
      LogTemplate *template = log_template_new(configuration, NULL);
      log_template_compile(template, left->template, &error);
      filter_match_set_template_ref(new_filter, template);
    }
  else
    filter_match_set_value_handle(new_filter, log_msg_get_value_handle(left->template));

  new_filter->comp = left->comp;
  new_filter->modify = left->modify;
  g_assert(!new_filter->modify);

  LogMatcherOptions *options = filter_re_get_matcher_options(new_filter);
  gboolean result = log_matcher_options_set_type(options, left->type);
  g_assert(result);


  filter_expr_replace_child(parent, current, new_filter);
  return new_filter;
}




static gboolean
_can_we_concatenate(FilterExprNode *current, FilterExprNode *left, FilterExprNode *right)
{
  // Is it an OR filter
  if (strcmp(current->type, "OR") != 0)
    return FALSE;

  if (left->comp != right->comp)
    return FALSE;

  if (strcmp(left->type, right->type) !=0 )
    return FALSE;

  //Glob is not trivial to convert to regex, nor scope
  if (strcmp(left->type, "glob") == 0)
    return FALSE;

  if (strcmp(left->template, right->template) !=0 )
    return FALSE;

  return TRUE;
}



static gpointer
_concatenate_or_filters_init(FilterExprNode *root)
{
  GList **stack = g_new0(GList *, 1);

  return stack;
}

static void
_concatenate_or_filters_deinit(gpointer cookie)
{
  GList **stack = (GList **)cookie;
  g_free(stack);
}

static void
_concatenate_or_filters_cb(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs, gpointer cookie)
{
  GList **stack = (GList **)cookie;

  GList *left_link = NULL;
  FilterExprNode *left = NULL;
  GList *right_link = NULL;
  FilterExprNode *right = NULL;
  if (strcmp(current->type, "OR") == 0)
    {
      left_link = g_list_last(*stack);
      right_link = g_list_last(*stack)->prev;
      left = (FilterExprNode *)left_link->data;
      right = (FilterExprNode *)right_link->data;
    }

  if (_can_we_concatenate(current, left, right))
    {
      *stack = g_list_remove_link(*stack, left_link);
      *stack = g_list_remove_link(*stack, right_link);

      *stack = g_list_append(*stack, _concatenate(current, parent, left, right));

      g_list_free_1(left_link);
      g_list_free_1(right_link);
    }
  else
    {
      *stack = g_list_append(*stack, current);
    }
}

FilterExprOptimizer concatenate_or_filters =
{
  .name = "concatenate-or-filters",
  .init = _concatenate_or_filters_init,
  .deinit = _concatenate_or_filters_deinit,
  .cb = _concatenate_or_filters_cb
};

FilterExprOptimizer *concatenate_or_filters_get_instance(void)
{
  return &concatenate_or_filters;
}

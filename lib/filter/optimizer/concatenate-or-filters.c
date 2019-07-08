#include "lib/filter/optimizer/concatenate-or-filters.h"
#include <stdio.h>

static void
_concatenate(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs)
{
  // TODO
  return;
}




static gboolean
_can_we_concatenate(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs)
{
  // Is it an OR filter
  if (strcmp(current->type, "OR") != 0)
    return FALSE;

  // OR filters always have two child element, left and right
  g_assert(childs->len == 2);
  FilterExprNode *left = (FilterExprNode *)g_ptr_array_index(childs, 0);
  FilterExprNode *right = (FilterExprNode *)g_ptr_array_index(childs, 1);

  // Currently we only concatenate if childs are not negated.
  if (!left->comp && !right->comp)
    return FALSE;

  // Different filters are concatenated differently, but they always have to have the same type.
  if (strcmp(left->type, right->type) !=0 )
    return FALSE;

  //.. TODO
  return TRUE;
}




static gpointer
_concatenate_or_filters_init(FilterExprNode *root)
{
  // I don't need a cookie for this job. So just return the root element,
  // as a valid not NULL pointer, and DO NOT free it up later :)
  return root;
}

static void
_concatenate_or_filters_deinit(gpointer cookie)
{
  return;
}

static void
_concatenate_or_filters_cb(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs, gpointer cookie)
{
  if (_can_we_concatenate(current, parent, childs))
    {
      _concatenate(current, parent, childs);
    }
}

FilterExprOptimizer concatenate_or_filters = {
  .name = "concatenate-or-filters",
  .init = _concatenate_or_filters_init,
  .deinit = _concatenate_or_filters_deinit,
  .cb = _concatenate_or_filters_cb
};

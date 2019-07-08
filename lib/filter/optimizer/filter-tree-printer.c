#include "filter/optimizer/filter-tree-printer.h"

static gpointer
_filter_tree_printer_init(FilterExprNode *root)
{
  gint *cookie = g_malloc0(sizeof(gint)); // indentation
  *cookie = 20;
  printf("%-*s%s\n", *cookie, "parent", "child(s)");
  return cookie;
}

static void
_filter_tree_printer_deinit(gpointer cookie)
{
  g_free(cookie);
}

static void
_filter_tree_printer_cb(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs, gpointer cookie)
{
  if (parent == NULL)
    printf("%-*s", *(gint*)cookie, "root");
  else
    printf("%-*s", *(gint*)cookie, parent->type);

  if (childs)
    {
      gint i;
      for (i = 0; i < childs->len; i++)
        {
          FilterExprNode *child = (FilterExprNode *)g_ptr_array_index(childs, i);
          printf("%s ", child->type);
        }
    }
  else
    {
      printf("leaf");
    }

  printf("\n");
}

FilterExprOptimizer filter_tree_printer = {
  .name = "filter-tree-printer",
  .init = _filter_tree_printer_init,
  .deinit = _filter_tree_printer_deinit,
  .cb = _filter_tree_printer_cb
};

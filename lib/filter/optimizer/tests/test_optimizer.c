#include <criterion/criterion.h>

#include "filter/filter-expr-parser.h"
#include "cfg-lexer.h"
#include "filter/optimizer/filter-expr-optimizer.h"
#include "filter/optimizer/concatenate-or-filters.h"
#include "apphook.h"


gint counter;

static gpointer
_dummy_init(FilterExprNode *root)
{
  ++counter;

  return (gpointer)&counter;
}

static void
_dummy_deinit(gpointer cookie)
{
  ++counter;
}

static void
_dummy_cb(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs, gpointer cookie)
{
  ++counter;
}

FilterExprOptimizer dummy = {
  .name = "dummy",
  .init =  _dummy_init,
  .deinit = _dummy_deinit,
  .cb = _dummy_cb
};


static FilterExprNode *
filter_dummy_new(void)
{
  FilterExprNode *self = g_new0(FilterExprNode, 1);

  filter_expr_node_init_instance(self);
  self->type = "dummy";

  return self;
}

static gpointer
_replace_init(FilterExprNode *root)
{
  return (gpointer)root;
}

static void
_replace_deinit(gpointer cookie)
{

}

static void
_replace_cb(FilterExprNode *current, FilterExprNode *parent, GPtrArray *childs, gpointer cookie)
{
  FilterExprNode *node = filter_dummy_new();
  filter_expr_replace_child(parent, current, node);
}

FilterExprOptimizer always_replace_with_dummy_filter = {
  .name = "replacer",
  .init =  _replace_init,
  .deinit = _replace_deinit,
  .cb = _replace_cb
};

static FilterExprNode* _compile_standalone_filter(gchar *config_snippet)
{
  GlobalConfig *cfg = cfg_new_snippet();
  CfgLexer *lexer = cfg_lexer_new_buffer(cfg, config_snippet, strlen(config_snippet));
  FilterExprNode *tmp;
  cr_assert(cfg_run_parser(cfg, lexer, &filter_expr_parser, (gpointer *) &tmp, NULL));

  return tmp;
}


Test(filter_optimizer, dummy_optimizer)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo');");

  cr_assert(filter_expr_optimizer_run(expr,  &dummy));
  cr_assert_eq(counter, 4, "%d==%d", counter, 4);

  app_shutdown();
}

Test(filter_optimizer, multiple_filter_expr)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo') and message('blaze');");

  cr_assert(filter_expr_optimizer_run(expr,  &dummy));
  cr_assert_eq(counter, 6);

  app_shutdown();
}


Test(replace_optimizer, simple_filter)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo');");

  FilterExprNode *result = filter_expr_optimizer_run(expr,  &always_replace_with_dummy_filter);

  cr_assert(result);
  cr_assert_eq(result->type, "dummy");

  app_shutdown();
}


Test(filter_optimizer, no_optimize)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo');");

  FilterExprNode *result = filter_expr_optimizer_run(expr,  &concatenate_or_filters);

  cr_assert_eq(expr, result);

  app_shutdown();
}

Test(filter_optimizer, same_filter_expr_with_and)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo') and program('boo');");

  FilterExprNode *result = filter_expr_optimizer_run(expr,  &concatenate_or_filters);

  cr_assert_eq(expr, result);

  app_shutdown();
}

Test(filter_optimizer, same_filter_expr_with_or)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo') or program('boo');");

  FilterExprNode *result = filter_expr_optimizer_run(expr,  &concatenate_or_filters);

  cr_assert_eq(expr, result);

  app_shutdown();
}




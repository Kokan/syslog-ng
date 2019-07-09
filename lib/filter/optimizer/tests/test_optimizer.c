#include <criterion/criterion.h>

#include "filter/filter-expr-parser.h"
#include "cfg-lexer.h"
#include "filter/optimizer/filter-expr-optimizer.h"
#include "filter/optimizer/concatenate-or-filters.h"


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
  cr_assert_eq(counter, 3, "%d==%d", counter, 3);

  app_shutdown();
}

Test(filter_optimizer, multiple_filter_expr)
{
  app_startup();
  FilterExprNode *expr = _compile_standalone_filter("program('foo') and message('blaze');");

  cr_assert(filter_expr_optimizer_run(expr,  &dummy));
  cr_assert_eq(counter, 5);

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

  cr_assert_neq(expr, result);

  app_shutdown();
}




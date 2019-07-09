#include <criterion/criterion.h>

#include "filter/filter-expr-parser.h"
#include "cfg-lexer.h"

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
  FilterExprNode *expr = _compile_standalone_filter("program('foo') or program('boo');");

  cr_assert(expr);
}



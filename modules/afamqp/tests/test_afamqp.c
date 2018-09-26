#include <criterion/criterion.h>

#include "afamqp.h"

Test(test_afamqp, create)
{
  LogDriver *dd = afamqp_dd_new(NULL);
  log_pipe_unref(&dd->super);
}

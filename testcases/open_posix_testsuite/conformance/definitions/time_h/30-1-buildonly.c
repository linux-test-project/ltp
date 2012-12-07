  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:TMR
   Test that the function:
   int timer_delete(timer_t);
   is declared.
   */

#include <time.h>

typedef int (*timer_delete_test) (timer_t);

int dummyfcn(void)
{
	timer_delete_test dummyvar;
	dummyvar = timer_delete;
	return 0;
}

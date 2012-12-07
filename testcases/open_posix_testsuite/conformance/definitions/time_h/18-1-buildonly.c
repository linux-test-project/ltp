  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   double difftime(time_t, time_t);
   is declared.
   */

#include <time.h>

typedef double (*difftime_test) (time_t, time_t);

int dummyfcn(void)
{
	difftime_test dummyvar;
	dummyvar = difftime;
	return 0;
}

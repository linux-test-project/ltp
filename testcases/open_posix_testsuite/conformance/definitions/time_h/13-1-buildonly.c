  /*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  @pt:TMR
  Test that the function:
   int clock_gettime(clockid_t, struct timespec *);
  is declared.
  */

#include <time.h>

typedef int (*clock_gettime_test)(clockid_t, struct timespec *);

int dummyfcn (void)
{
	clock_gettime_test dummyvar;
	dummyvar = clock_gettime;
	return 0;
}

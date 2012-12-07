  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:CPT
   Test that the function:
   int clock_getcpuclockid(pid_t, clockid_t *);
   is declared.
   */

#include <time.h>

typedef int (*clock_getcpuclockid_test) (pid_t, clockid_t *);

int dummyfcn(void)
{
	clock_getcpuclockid_test dummyvar;
	dummyvar = clock_getcpuclockid;
	return 0;
}

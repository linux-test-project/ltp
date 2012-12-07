  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:TSF
   Test that the function:
   char *ctime_r(const time_t *, char *);
   is declared.
   */

#include <time.h>

typedef char *(*ctime_r_test) (const time_t *, char *);

int dummyfcn(void)
{
	ctime_r_test dummyvar;
	dummyvar = ctime_r;
	return 0;
}

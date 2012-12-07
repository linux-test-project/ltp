  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:TSF
   Test that the function:
   char *asctime_r(const struct tm *, char *);
   is declared.
   */

#include <time.h>

typedef char *(*asctime_r_test) (const struct tm *, char *);

int dummyfcn(void)
{
	asctime_r_test dummyvar;
	dummyvar = asctime_r;
	return 0;
}

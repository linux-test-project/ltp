  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   time_t mktime(struct tm *);
   is declared.
   */

#include <time.h>

typedef time_t(*mktime_test) (struct tm *);

int dummyfcn(void)
{
	mktime_test dummyvar;
	dummyvar = mktime;
	return 0;
}

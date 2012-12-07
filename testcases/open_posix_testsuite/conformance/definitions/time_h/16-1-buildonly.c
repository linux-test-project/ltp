  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   char *ctime(const time_t *);
   is declared.
   */

#include <time.h>

typedef char *(*ctime_test) (const time_t *);

int dummyfcn(void)
{
	ctime_test dummyvar;
	dummyvar = ctime;
	return 0;
}

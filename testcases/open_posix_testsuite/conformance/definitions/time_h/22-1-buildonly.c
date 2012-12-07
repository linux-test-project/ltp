  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   struct tm *localtime(const time_t *);
   is declared.
   */

#include <time.h>

typedef struct tm *(*localtime_test) (const time_t *);

int dummyfcn(void)
{
	localtime_test dummyvar;
	dummyvar = localtime;
	return 0;
}

  /*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   @pt:TSF
  Test that the function:
   struct tm *gmtime_r(const time_t *, struct tm *);
  is declared.
  */

#include <time.h>

typedef struct tm *(*gmtime_r_test)(const time_t *, struct tm *);

int dummyfcn (void)
{
	gmtime_r_test dummyvar;
	dummyvar = gmtime_r;
	return 0;
}

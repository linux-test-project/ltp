  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   size_t strftime(char *, size_t, const char *, const struct tm *);
   is declared.
   */

#include <time.h>

typedef size_t(*strftime_test) (char *, size_t, const char *,
				const struct tm *);

int dummyfcn(void)
{
	strftime_test dummyvar;
	dummyvar = strftime;
	return 0;
}

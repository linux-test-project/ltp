  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   @pt:XSI
   Test that the function:
   char *strptime(const char *, const char *, struct tm *);
   is declared.
   */

#include <time.h>

typedef char *(*strptime_test) (const char *, const char *, struct tm *);

int dummyfcn(void)
{
	strptime_test dummyvar;
	dummyvar = strptime;
	return 0;
}

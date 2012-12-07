  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   void tzset(void);
   is declared.
   */

#include <time.h>

typedef void (*tzset_test) (void);

int dummyfcn(void)
{
	tzset_test dummyvar;
	dummyvar = tzset;
	return 0;
}

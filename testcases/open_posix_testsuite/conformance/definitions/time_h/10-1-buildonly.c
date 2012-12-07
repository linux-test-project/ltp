  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test that the function:
   clock_t clock(void);
   is declared.
   */

#include <time.h>

typedef clock_t(*clock_test) (void);

int dummyfcn(void)
{
	clock_test dummyvar;
	dummyvar = clock;
	return 0;
}

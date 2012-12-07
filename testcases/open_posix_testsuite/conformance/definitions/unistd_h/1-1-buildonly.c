  /*
   * Copyright (c) 2003, Intel Corporation. All rights reserved.
   * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.
   *
   * Test that the function below is defined:
   * int fsync(int fildes);
   */

#include <unistd.h>

typedef int (*fsync_test) (int fildes);

int dummyfcn(void)
{
	fsync_test dummyvar;
	dummyvar = fsync;
	return 0;
}

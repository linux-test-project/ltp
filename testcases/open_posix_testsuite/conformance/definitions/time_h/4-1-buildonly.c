/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that timespec structure is declared.
 */
#include <time.h>

struct timespec this_type_should_exist, t;

int dummyfcn(void)
{
	time_t sec;
	long nsec;

	sec = t.tv_sec;
	nsec = t.tv_nsec;
	return 0;
}

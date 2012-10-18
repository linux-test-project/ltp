/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that itimerspec structure is declared.
 */
#include <time.h>

struct itimerspec this_type_should_exist, t;

int dummyfcn(void)
{
	struct timespec interval;
	struct timespec value;

	interval = t.it_interval;
	value = t.it_value;
	return 0;
}

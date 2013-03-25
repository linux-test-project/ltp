/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test to see if timer_getoverrun() sets errno=EINVAL if no timers have been
 * created yet or not.  Since this is a "may" assertion, either way is
 * a pass.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define BOGUSTID 9999

int main(void)
{
	timer_t tid;
	int tval = BOGUSTID;
	tid = (timer_t) & tval;

	if (timer_getoverrun(tid) == -1) {
		if (EINVAL == errno) {
			printf("fcn returned -1 and errno=EINVAL\n");
			return PTS_PASS;
		} else {
			printf("fcn returned -1, but errno!=EINVAL\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("fcn did not return -1\n");
	return PTS_PASS;
}

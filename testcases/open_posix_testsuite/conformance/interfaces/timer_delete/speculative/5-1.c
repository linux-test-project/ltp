/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test to see if timer_delete() returns -1 and sets errno==EINVAL if
 * timerid is not a valid timer ID or not.
 * Since this is a "may" requirement, either option is a PASS.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define BOGUSTIMERID 99999

int main(void)
{
	timer_t tid;
	int tval = BOGUSTIMERID;
	tid = (timer_t) & tval;

	if (timer_delete(tid) == -1) {
		if (errno == EINVAL) {
			printf
			    ("timer_delete() returned -1 and set errno=EINVAL\n");
			return PTS_PASS;
		} else {
			printf
			    ("timer_delete() returned -1, but didn't set errno!=EINVAL\n");
			return PTS_FAIL;
		}
	}

	printf("timer_delete() did not return -1\n");
	return PTS_PASS;
}

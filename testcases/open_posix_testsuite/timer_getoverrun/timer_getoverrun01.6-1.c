/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test that timer_getoverrun() sets errno=EINVAL if no timers have been
 * created yet.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


#define BOGUSTID 9999

int main(int argc, char *argv[])
{
	timer_t tid;

	tid = BOGUSTID;

	if (timer_getoverrun(tid) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("returned -1, but errno not set\n");
			return PTS_FAIL;
		}
	} else {
		printf("timer_getoverrun() did not return failure\n");
		return PTS_UNRESOLVED;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}

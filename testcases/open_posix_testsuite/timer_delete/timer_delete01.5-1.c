/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_delete() returns -1 and sets errno==EINVAL if
 * timerid is not a valid timer ID.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define BOGUSTIMERID 99999

int main(int argc, char *argv[])
{
	timer_t tid;

	tid = BOGUSTIMERID;

	if (timer_delete(tid) == -1) {
		if (errno==EINVAL) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno!=EINVAL when bogus timer ID sent\n");
			return PTS_FAIL;
		}
	} else {
		printf("timer_delete() did not return -1 for bogus timer ID\n");
		return PTS_FAIL;
	}

	printf("This code should never be executed\n");
	return PTS_UNRESOLVED;
}

/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the killpg() function shall set errno to ESRCH if it is
    passed an invalid process group number

 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	if (killpg(999999, 0) != -1) {
		printf
		    ("killpg did not return -1 even though it was passed an invalid process group id.");
		return PTS_UNRESOLVED;
	}

	if (errno != ESRCH) {
		printf
		    ("killpg did not set errno to ESRCH even though it was passed an invalid signal number.");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

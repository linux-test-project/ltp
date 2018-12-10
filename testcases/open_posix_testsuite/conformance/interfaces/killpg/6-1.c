/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the killpg() function shall set errno to EINVAL if it is
    passed an invalid signal number

 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int pgrp;

	if ((pgrp = getpgrp()) == -1) {
		printf("Could not get process group number\n");
		return PTS_UNRESOLVED;
	}

	if (killpg(pgrp, -1) != -1) {
		printf
		    ("killpg did not return -1 even though it was passed an invalid signal number.");
		return PTS_UNRESOLVED;
	}

	if (errno != EINVAL) {
		printf
		    ("killpg did not set errno to EINVAL even though it was passed an invalid signal number.");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

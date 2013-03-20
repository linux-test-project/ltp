/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if the user id of the sending process
 doesn't match the user id of the receiving process (pid), then the kill function
 will fail with errno set to EPERM.

 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "posixtest.h"

int main(void)
{
	/* this is added incase user is root. If user is normal user, then it
	 * has no effect on the tests */
	setuid(1);

	if (kill(1, 0) != -1) {
		printf
		    ("Test FAILED: kill() succeeded even though this program's user id did not match the recieving process's user id\n");
		return PTS_FAIL;
	}

	if (EPERM != errno) {
		printf("Test FAILED: EPERM error not received\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

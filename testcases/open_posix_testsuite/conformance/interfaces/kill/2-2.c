#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "posixtest.h"

/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that when the null signal is sent to kill(), error checking is
 *  still performed.
 *  1) Send a signal to generate an ESRCH error.
 *     ==> Send a signal to PID 999999
 *  2) Verify ESRCH error received and kill() returned -1.
 *  3) Send a signal to generate an EPERM error.
 *     ==> Set UID to 1 and send a signal to init (pid = 1)
 *  4) Verify EPERM error received and kill() returned -1.
 *
 *  Note:  These tests make the assumptions that:
 *         - They will be running as root.
 *         - The PID 999999 can never exist.
 *         - The UID 1 is available for assignment and cannot sent
 *           signals to root.
 *         *** I need to check to see if these assumptions are always valid.
 */

int main(void)
{
	int failure = 0;

	/*
	 * ESRCH
	 */
	if (-1 == kill(999999, 0)) {
		if (ESRCH == errno) {
			printf("ESRCH error received\n");
		} else {
			printf
			    ("kill() failed on ESRCH errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("kill() did not fail on ESRCH\n");
		failure = 1;
	}

	/*
	 * EPERM
	 */
	/* this is added incase user is root. If user is normal user, then it
	 * has no effect on the tests */
	setuid(1);

	if (-1 == kill(1, 0)) {
		if (EPERM == errno) {
			printf("EPERM error received\n");
		} else {
			printf
			    ("kill() failed on EPERM errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("kill() did not fail on EPERM\n");
		failure = 1;
	}

	if (failure) {
		printf("At least one test FAILED -- see output for status\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}

/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Attempt to add SIGKILL and SIGSTOP to the process's signal mask and
 verify that:
 - They do not get added.
 - sigprocmask() does not return -1.
*/

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	sigset_t set1, set2;
	int sigprocmask_return_val = 1;

	sigemptyset(&set1);
	sigemptyset(&set2);
	sigaddset(&set1, SIGKILL);
	sigaddset(&set1, SIGSTOP);
	sigprocmask_return_val = sigprocmask(SIG_SETMASK, &set1, NULL);
	sigprocmask(SIG_SETMASK, NULL, &set2);

	if (sigismember(&set2, SIGKILL)) {
		printf("FAIL: SIGKILL was added to the signal mask\n");
		return PTS_FAIL;
	}
	if (sigismember(&set2, SIGSTOP)) {
		printf("FAIL: SIGSTOP was added to the signal mask\n");
		return PTS_FAIL;
	}
	if (sigprocmask_return_val == -1) {
		printf
		    ("FAIL: sigprocmask returned -1. System should be able to enforce blocking un-ignorable signals without causing sigprocmask() to return -1.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

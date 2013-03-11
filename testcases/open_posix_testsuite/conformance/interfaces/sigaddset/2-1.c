/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test adding signo to a signal set without first calling
   sigemptyset() or sigfillset().
   Results are undefined; however, we should definitely not see system
   crashes or hangs or other equally harmful behavior.
   So, if the system is able to return test results, then this test case
   passes.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if (sigaddset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 1) {
			printf("Test PASSED: Signal was added\n");
			return PTS_PASS;
		}
		printf("Signal was not added\n");
		return PTS_FAIL;
	}

	printf("sigaddset did not return 0\n");
	return PTS_FAIL;
}

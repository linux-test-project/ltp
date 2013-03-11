/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that sigaddset() will add signo to the set signal set.
 * Test steps:
 * 1)  Initialize an empty or full signal set.
 * 2)  Add the SIGALRM signal to the empty signal set.
 * 3)  Verify that SIGALRM is a member of the signal set.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if (sigfillset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 1) {
			printf("Test PASSED: sigaddset added signal\n");
			return PTS_PASS;
		}
		printf("sigaddset returned, sigmember failed\n");
		return PTS_FAIL;
	}

	printf("sigaddset failed\n");
	return PTS_FAIL;
}

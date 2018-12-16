/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if signal has been blocked, then
 sigset shall return SIG_HOLD

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
	sigset_t st;
	sigemptyset(&st);
	sigaddset(&st, SIGCHLD);

	if (sigprocmask(SIG_BLOCK, &st, NULL) < 0) {
		printf("Test FAILED: sigprocmask(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (sigset(SIGCHLD, SIG_HOLD) != SIG_HOLD) {
		printf("Test FAILED: sigset() didn't return SIG_HOLD\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}

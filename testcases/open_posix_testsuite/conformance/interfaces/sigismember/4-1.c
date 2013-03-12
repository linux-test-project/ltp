/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Tests assertion 4 by emptying a signal set and querying it for
 * a SIGABRT function. Sigmember should return a 0 indicating that
 * the signal is not a member of the set.
 */

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&signalset, SIGABRT) != 0) {
		printf("sigismember dit not return 0");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

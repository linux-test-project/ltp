#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that if the signal is the null signal (0), no signal is sent.
 *  1) Call kill on the current process with the null signal.
 *  2) If process is still functional after kill() is called, consider
 *     the test a pass (most likely no signal was sent).
 */

int main(void)
{
	if (kill(getpid(), 0) != 0) {
		printf("Could not call kill with sig = 0\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

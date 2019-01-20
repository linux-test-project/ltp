/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that signal() shall return SIG_ERR
 and set errno to a positive value if an invalid signal number was
 passed to it.

*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("handler does nothing useful.\n");
}

int main(void)
{
	errno = -1;

	if (signal(SIGKILL, myhandler) != SIG_ERR) {
		printf
		    ("Test FAILED: signal() didn't return SIG_ERR even though a non-catchable signal was passed to it\n");
		return PTS_FAIL;
	}

	if (errno <= 0) {
		printf
		    ("Test FAILED: errno wasn't set to a positive number even though a non-catchable signal was passed to the signal() function\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}

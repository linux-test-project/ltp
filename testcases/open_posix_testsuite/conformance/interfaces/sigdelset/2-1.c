/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by: julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that the results are undefined if sigdelset() is called without
   first calling sigemptyset() or sigfillset().
   Any results are acceptable; however, the system should not crash, hang,
   or do something equally as harmful.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
	sigset_t signalset;

	if (sigdelset(&signalset, SIGALRM) == 0) {
		printf("sigdelset returned 0\n");
	} else {
		printf("sigdelset() did not return 0\n");
	}
	/*
	 * If we made it here, the test case passes.
	 */
	return PTS_PASS;
}
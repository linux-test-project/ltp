/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *
 *  Test that the results are undefined if sigismember() is called without
 *  first calling sigemptyset() or sigfillset().
 *  Any results are acceptable; however, the system should not crash, hang,
 *  or do something equally as harmful.
 */

#include <stdio.h>
#include <signal.h>
#include <posixtest.h>

int main()
{
	sigset_t signalset;
	int returnval;
	returnval = sigismember(&signalset, SIGALRM);
	(void)returnval;

#ifdef DEBUG
	printf("sigismember returned returnval\n");
#endif

	/*
	 * If we made it here, the test case passes.
	 */
	return PTS_PASS;
}

/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
   Testing sending invalid signals to sigaddset().
   After invalid signal sent, sigaddset() should return -1 and set
   errno to indicate the error.
   Test steps:
   1)  Initialize an empty signal set.
   2)  Add the invalid signal to the empty signal set.
   3)  Verify that -1 is returned, the invalid signal is not a member of
       the signal set, and errno is set to indicate the error.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
	sigset_t signalset;
	int signo;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	if (argc < 2) {
        	printf("Usage:  %s [1|2|3|4]\n", argv[0]);
		return PTS_UNRESOLVED;
	}

	/*
		Various error conditions
	*/
	switch (argv[1][0]) {
		case '1':
			signo=-1;
			break;
		case '2':
			signo=-10000;
			break;
		case '3':
			signo=INT32_MIN+1;
			break;
		case '4':
			signo=INT32_MIN;
			break;
		default:
			printf("Usage:  %s [1|2|3|4]\n", argv[0]);
			return PTS_UNRESOLVED;
	}

	if (sigaddset(&signalset, signo) == -1) {
		if (EINVAL == errno) {
			printf ("errno set to EINVAL\n");
			if ((sigismember(&signalset, signo) == 0) ||
				sigismember(&signalset, signo) == -1) {
				printf("signal was not added\n");
				return PTS_PASS;
			} else {
				printf("possible error in sigismember\n");
				return PTS_UNRESOLVED;
			}
		} else {
			printf ("errno not set to EINVAL\n");
			return PTS_FAIL;
		}
	}

	printf("sigaddset did not return -1\n");
	return PTS_FAIL;
}
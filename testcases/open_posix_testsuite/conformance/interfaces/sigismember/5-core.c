/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *

   Testing sending invalid signals to sigdelset().
   After invalid signal sent, sigdelset() should return -1 and set
   errno to indicate the error.
   Test steps:
   1)  Initialize a full signal set.
   2)  Remove the invalid signal from the full signal set.
   3)  Verify that -1 is returned, the invalid signal is not a member of
       the signal set, and errno is set to indicate the error.
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <posixtest.h>

int main(int argc, char *argv[])
{
	sigset_t signalset;
	int signo;

	if (sigfillset(&signalset) == -1) {
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

	if (sigismember(&signalset, signo) != -1) {
		printf("sigismember did not return -1\n");
		return PTS_FAIL;
	} else if (EINVAL != errno) {
		printf("errno was not set to EINVAL\n");
		return PTS_FAIL;
	}

	printf ("errno set to EINVAL and sigismember returned -1\n");
	return PTS_PASS;
}

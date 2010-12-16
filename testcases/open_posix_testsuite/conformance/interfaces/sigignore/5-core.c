/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Testing passing an invalid signals to sighold().
   After sighold is called on an invalid signal, sigignore() should
return -1 and set errno to EINVAL

   The invalid signal passed to sigignore() depends on the argument
passed to this program. There are currently 4 invalid signals.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
	int signo;

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

	if (sigignore(signo) == -1) {
		if (EINVAL == errno) {
			printf ("errno set to EINVAL\n");
			return PTS_PASS;
		} else {
			printf ("errno not set to EINVAL\n");
			return PTS_FAIL;
		}
	}

	printf("sighold did not return -1\n");
	return PTS_FAIL;
}
/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * @pt:CPT
 * General test that clock_getcpuclockid() returns CPU-time clock for a 
 * process (the process chosen is init -- pid = 1).
 *
 *
 * 12/17/02 - Checking in correction made by
 *            jim.houston REMOVE-THIS AT attbi DOT com
 *            Test needed to do something as opposed to idle sleep to
 *            get the CPU time to increase.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define LARGENUMBER 900000
void dosomething()
{
	int i;
	for (i=0; i < LARGENUMBER; i++) {
		clock();
	}
}

int main(int argc, char *argv[])
{
	clockid_t clockid;

	dosomething();
	if (clock_getcpuclockid(1, &clockid) == 0) {
		if (&clockid != NULL) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("clock_getcpuclockid returned NULL\n");
			return PTS_FAIL;
		}
	} else {
		printf("clock_getcpuclockid() failed\n");
		return PTS_FAIL;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}

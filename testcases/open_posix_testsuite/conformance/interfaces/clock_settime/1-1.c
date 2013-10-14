/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_settime() sets clock_id to tp.
 *
 * The clock_id chosen for this test is CLOCK_REALTIME.
 * The date chosen is Nov 12, 2002 ~11:13am (date when test was first
 * written).
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"
#include "helpers.h"

#ifndef PR_NSEC_PER_SEC
#define PR_NSEC_PER_SEC 1000000000UL
#endif

#define TESTTIME 1037128358
#define ACCEPTABLEDELTA 1

int main(void)
{
	struct timespec tpset, tpget, tpreset, tpres;
	int delta, nsdelta;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (getuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}
	if (clock_getres(CLOCK_REALTIME, &tpres) != 0) {
		printf("Time resolution is not provided\n");
		tpres.tv_sec = 0;
		tpres.tv_nsec = 10000000;
	}

	getBeforeTime(&tpreset);

	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;
	if (clock_settime(CLOCK_REALTIME, &tpset) == 0) {
		if (clock_gettime(CLOCK_REALTIME, &tpget) == -1) {
			printf("Error in clock_gettime()\n");
			setBackTime(tpreset);
			return PTS_UNRESOLVED;
		}
		delta = tpget.tv_sec - tpset.tv_sec;
		nsdelta = PR_NSEC_PER_SEC - tpget.tv_nsec;
		if ((delta <= ACCEPTABLEDELTA) && (delta >= 0)) {
			printf("Test PASSED\n");
			setBackTime(tpreset);
			return PTS_PASS;
		} else if ((nsdelta <= tpres.tv_nsec) && (delta == -1)) {
			printf("Test PASSED\n");
			setBackTime(tpreset);
			return PTS_PASS;
		} else {
			printf("clock does not appear to be set\n");
			setBackTime(tpreset);
			return PTS_FAIL;
		}
	}

	printf("clock_settime() failed\n");
	setBackTime(tpreset);
	return PTS_UNRESOLVED;
}

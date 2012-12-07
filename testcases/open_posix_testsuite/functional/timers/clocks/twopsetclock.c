/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test having two processes attempt to set the clock at the same time.
 * Ensure that both actually set the clock, and it is the later one that
 * takes effect.  [Note:  It would be hard to test that they both set
 * the clock without setting up atomic operations.  Will just test that
 * at least one set took place.]
 * The two processes will attempt to set the clock to TESTTIME+DELTA
 * and TESTTIME-DELTA.
 *
 * The clock_id chosen for this test is CLOCK_REALTIME.
 * The date chosen is Nov 12, 2002 ~11:13am.
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define TESTTIME 1037128358
#define DELTA 5
#define ACCEPTABLEDELTA 1
#define LONGTIME 3		//== long enough for both clocks to be set

int main(int argc, char *argv[])
{
	struct timespec tpget, tsreset;
	int pid, delta;

	if (clock_gettime(CLOCK_REALTIME, &tsreset) != 0) {
		perror("clock_getime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/*child */
		struct timespec tschild;

		tschild.tv_sec = TESTTIME + DELTA;
		tschild.tv_nsec = 0;
		if (clock_settime(CLOCK_REALTIME, &tschild) != 0) {
			printf("Note:  clock_settime() failed\n");
		}
		if (clock_gettime(CLOCK_REALTIME, &tpget) == -1) {
			printf("Note:  Error in clock_gettime()\n");
		}

	} else {
		/*parent */
		struct timespec tsparent;
		int pass = 0;

		tsparent.tv_sec = TESTTIME - DELTA;
		tsparent.tv_nsec = 0;
		if (clock_settime(CLOCK_REALTIME, &tsparent) != 0) {
			printf("Note:  clock_settime() failed\n");
		}

		sleep(LONGTIME);

		/*
		 * Ensure we set clock to TESTTIME-DELTA or TESTTIME+DELTA.
		 * Assume that clock increased monotonically and clock_gettime,
		 * clock_settime return correct values.
		 */

		if (clock_gettime(CLOCK_REALTIME, &tpget) == -1) {
			printf("Note:  Error in clock_gettime()\n");
		}

		delta = (tpget.tv_sec - LONGTIME) - TESTTIME;

		if ((delta <= ACCEPTABLEDELTA - DELTA) ||
		    (delta <= ACCEPTABLEDELTA + DELTA)) {
			pass = 1;
		}

		if (clock_settime(CLOCK_REALTIME, &tsreset) != 0) {
			printf("Need to manually reset time\n");
		}

		if (pass) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	return PTS_UNRESOLVED;
}

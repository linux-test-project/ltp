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
#include "posixtest.h"
#include "helpers.h"

#define TESTTIME 1037128358
#define ACCEPTABLEDELTA 1

int main(int argc, char *argv[])
{
	struct timespec tpset, tpget, tpreset;
	int delta;

	getBeforeTime(&tpreset);

	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;
	if (clock_settime(CLOCK_REALTIME, &tpset) == 0) {
		if (clock_gettime(CLOCK_REALTIME, &tpget) == -1) {
			printf("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}
		delta = tpget.tv_sec-tpset.tv_sec;
		if ( (delta <= ACCEPTABLEDELTA) && (delta >= 0) ) {
			printf("Test PASSED\n");
			setBackTime(tpreset);
			return PTS_PASS;
		} else {
			printf("clock does not appear to be set\n");
			return PTS_FAIL;
		}
	} else {
		printf("clock_settime() failed\n");
		return PTS_UNRESOLVED;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}

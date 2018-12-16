/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Simply, if sigrelse returns a 0 here, test passes.
 *
 */

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	if (sigrelse(SIGABRT) != 0) {
		perror("Sigrelse failed");
		return PTS_UNRESOLVED;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}

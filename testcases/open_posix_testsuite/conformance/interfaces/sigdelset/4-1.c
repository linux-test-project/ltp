/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2013, Cyril Hrubis <chrubis@suse.cz>
 *
 * Created by: julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Testing sending invalid signals to sigdelset().
 * After invalid signal sent, sigdelset() should return -1 and set
 * errno to indicate the error.
 * Test steps:
 * 1)  Initialize a full signal set.
 * 2)  Remove the invalid signal from the full signal set.
 * 3)  Verify that -1 is returned, the invalid signal is not a member of
 *     the signal set, and errno is set to indicate the error.
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

static const int sigs[] = {-1, -10000, INT32_MIN, INT32_MIN + 1};

#define	NUMSIGNALS	(sizeof(sigs) / sizeof(sigs[0]))

int main(void)
{
	sigset_t signalset;
	int i, ret, err = 0;

	if (sigfillset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < (int)NUMSIGNALS; i++) {
		ret = sigdelset(&signalset, sigs[i]);
		if (ret != -1 || errno != EINVAL) {
			err++;
			printf("Failed sigdelset(..., %i) ret=%i errno=%i\n",
			       sigs[i], ret, errno);
		}
	}

	if (err) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}

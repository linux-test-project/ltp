/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2013, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * Testing sending invalid signals to sigaddset().
 * After invalid signal set, sigaddset() should return -1 and set
 *  errno to indicate the error.
 * Test steps:
 * 1)  Initialize an empty signal set.
 * 2)  Add the invalid signal to the empty signal set.
 * 3)  Verify that -1 is returned, the invalid signal is not a member of
 *     the signal set, and errno is set to indicate the error.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include "posixtest.h"

static const int sigs[] = {-1, -10000, INT32_MIN, INT32_MIN + 1};

int main(void)
{
	sigset_t signalset;
	int ret, err = 0;
	unsigned int i;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < ARRAY_SIZE(sigs); i++) {
		ret = sigaddset(&signalset, sigs[i]);

		if (ret != -1 || errno != EINVAL) {
			err++;
			printf("Failed sigaddset(..., %i) ret=%i errno=%i\n",
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

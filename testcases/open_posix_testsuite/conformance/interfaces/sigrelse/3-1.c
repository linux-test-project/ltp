/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Copyright (c) 2013, Cyril Hrubis <chrubis@suse.cz>
 *
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Testing passing an invalid signals to sigrelse().
 * After sighold is called on an invalid signal, sigrelse() should
 * return -1 and set errno to EINVAL
 *
 * The invalid signal passed to sigrelse() depends on the argument
 * passed to this program.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <setjmp.h>
#include "posixtest.h"

static const int sigs[] = {-1, -10000, INT32_MIN, INT32_MIN + 1};

#define	NUMSIGNALS	(sizeof(sigs) / sizeof(sigs[0]))

int main(void)
{
	int i, ret, err = 0;

	for (i = 0; i < (int)NUMSIGNALS; i++) {
		ret = sigrelse(sigs[i]);

		if (ret != -1 || errno != EINVAL) {
			err++;
			printf("Failed sigrelse(%i) ret=%i errno=%i\n",
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

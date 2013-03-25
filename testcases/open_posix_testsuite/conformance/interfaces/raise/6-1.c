/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the raise(<signal>) function sends a non-zero value on
 *  unsuccessful completion.
 *  1) Raise an invalid signal.
 *  2) Verify a non-zero value is returned.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	if (raise(10000) == 0) {
		printf("Incorrectly returned 0\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

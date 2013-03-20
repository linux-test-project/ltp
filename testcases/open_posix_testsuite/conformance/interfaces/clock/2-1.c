/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * @pt:XSI
 * Test that CLOCKS_PER_SEC == 1,000,000 in <time.h>
 */

// Applied patch from Craig Rodrigues; no longer assumes CLOCKS_PER_SEC is long
// 12-18-02 Per suggestion by neal REMOVE-THIS AT cs DOT uml DOT edu started
// using intmax_h and INTMAX_C.  Also added use of PRIdMAX per his suggestion.

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "posixtest.h"

#define EXPECTEDVALUE INTMAX_C(1000000)

int main(void)
{
	intmax_t clocks_per_sec = (intmax_t) CLOCKS_PER_SEC;

	if (EXPECTEDVALUE == CLOCKS_PER_SEC) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("FAIL:  %" PRIdMAX " != %" PRIdMAX "\n",
		       clocks_per_sec, EXPECTEDVALUE);
		return PTS_FAIL;
	}

	return PTS_UNRESOLVED;
}

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*\
 * - Basic test for the time(2) system call. It is intended to provide a
 * limited exposure of the system call.
 * - Verify that time(2) returns the value of time in seconds since the Epoch
 * and stores this value in the memory pointed to by the parameter.
 */

#include <time.h>
#include <errno.h>

#include "tst_test.h"

static time_t tlocal;
static time_t *targs[] = {
	NULL, &tlocal,
};

static void verify_time(unsigned int i)
{
	time_t *tloc = targs[i];

	TEST(time(tloc));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "time()");
		return;
	}

	if (!tloc) {
		tst_res(TPASS, "time() returned value %ld", TST_RET);
	} else if (*tloc == TST_RET) {
		tst_res(TPASS,
			"time() returned value %ld, stored value %jd are same",
			TST_RET, (intmax_t) *tloc);
	} else {
		tst_res(TFAIL,
			"time() returned value %ld, stored value %jd are different",
			TST_RET, (intmax_t) *tloc);
	}
}

static struct tst_test test = {
	.test = verify_time,
	.tcnt = ARRAY_SIZE(targs),
};

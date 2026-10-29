// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2001-2024
 */

/*\
 * Check the basic functionality of settimeofday().
 */

#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define	VAL_SEC		100
#define	VAL_MSEC	100
#define ACCEPTABLE_DELTA 500
#define USEC_PER_SEC    1000000L

static void verify_settimeofday(void)
{
	suseconds_t delta;
	struct timeval tv1, tv2;

	if (gettimeofday(&tv1, NULL) == -1)
		tst_brk(TBROK | TERRNO, "gettimeofday(&tv1, NULL) failed");

	tv1.tv_sec += VAL_SEC;
	tv1.tv_usec += VAL_MSEC;
	if (tv1.tv_usec >= USEC_PER_SEC)
		tv1.tv_usec = VAL_MSEC;

	TEST(settimeofday(&tv1, NULL));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "settimeofday(&tv1, NULL) failed");
		return;
	}

	if (gettimeofday(&tv2, NULL) == -1)
		tst_brk(TBROK | TERRNO, "gettimeofday(&tv2, NULL) failed");

	if (tv2.tv_sec > tv1.tv_sec) {
		delta =
			(suseconds_t) (tv2.tv_sec - tv1.tv_sec) * 1000 +
			(tv2.tv_usec - tv1.tv_usec) / 1000;
	} else {
		delta =
			(suseconds_t) (tv1.tv_sec - tv2.tv_sec) * 1000 +
			(tv1.tv_usec - tv2.tv_usec) / 1000;
	}

	if (delta > -ACCEPTABLE_DELTA && delta < ACCEPTABLE_DELTA)
		tst_res(TPASS, "settimeofday() pass");
	else
		tst_res(TFAIL, "settimeofday() fail");
}

static struct tst_test test = {
	.restore_wallclock = 1,
	.test_all = verify_settimeofday,
	.needs_root = 1,
};

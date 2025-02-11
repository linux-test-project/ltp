// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */
/*\
 * This is a Phase I test for the times(2) system call.  It is intended to
 * provide a limited exposure of the system call.
 */

#include <sys/times.h>
#include <errno.h>
#include "tst_test.h"

static void verify_times(void)
{
	struct tms mytimes;

	TEST(times(&mytimes));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "times failed");
	else
		tst_res(TPASS, "times(&mytimes) returned %ld", TST_RET);
}

static struct tst_test test = {
	.test_all = verify_times,
};

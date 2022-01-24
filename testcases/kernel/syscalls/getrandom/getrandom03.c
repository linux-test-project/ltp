// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cedric Hnyda <ced.hnyda@gmail.com>
 *
 * Calls getrandom(2), check that the return value is equal to the
 * number of bytes required and expects success.
 */

#include "tst_test.h"
#include "lapi/getrandom.h"
#include "lapi/syscalls.h"

#define MAX_SIZE 256

static unsigned int sizes[] = {
	1,
	2,
	3,
	7,
	8,
	15,
	22,
	64,
	127,
};

static void verify_getrandom(unsigned int n)
{
	char buf[MAX_SIZE];

	TEST(tst_syscall(__NR_getrandom, buf, sizes[n], 0));

	if (TST_RET != sizes[n]) {
		tst_res(TFAIL | TTERRNO, "getrandom returned %li, expected %u",
			TST_RET, sizes[n]);
	} else {
		tst_res(TPASS, "getrandom returned %ld", TST_RET);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(sizes),
	.test = verify_getrandom,
};

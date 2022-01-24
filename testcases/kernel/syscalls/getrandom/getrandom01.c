// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cedric Hnyda <ced.hnyda@gmail.com>
 *
 * Calls getrandom(2) with a NULL buffer and expects failure.
 */

#include "tst_test.h"
#include "lapi/getrandom.h"
#include "lapi/syscalls.h"

static int modes[] = {0, GRND_RANDOM, GRND_NONBLOCK,
		      GRND_RANDOM | GRND_NONBLOCK};

static void verify_getrandom(unsigned int n)
{
	TEST(tst_syscall(__NR_getrandom, NULL, 100, modes[n]));

	if (TST_RET == -1) {
		tst_res(TPASS | TTERRNO, "getrandom returned %ld",
			TST_RET);
	} else {
		tst_res(TFAIL | TTERRNO, "getrandom failed");
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(modes),
	.test = verify_getrandom,
};

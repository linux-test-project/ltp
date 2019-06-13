// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018, Linux Test Project
 */

#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static void run(void)
{
	do {
		sleep(1);
	} while (tst_timeout_remaining() >= 4);

	tst_res(TPASS, "Timeout remaining: %d", tst_timeout_remaining());
}

static struct tst_test test = {
	.test_all = run,
	.timeout = 5
};

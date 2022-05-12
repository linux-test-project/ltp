// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, Linux Test Project
 */
/*
 * This test is set up so that the timeout is not long enough to guarantee
 * enough runtime for two iterations, i.e. the timeout without offset and after
 * scaling is too small and the tests ends up with TBROK.
 *
 * The default timeout in the test library is set to 30 seconds. The test
 * runtime is set to 5 so the test should timeout after 35 seconds.
 */

#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static void run(void)
{
	tst_res(TINFO, "Sleeping for 40 seconds");
	sleep(40);
	tst_res(TFAIL, "Still alive");
}

static struct tst_test test = {
	.test_all = run,
	.max_runtime = 5,
};

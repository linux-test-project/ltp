// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018, Linux Test Project
 *
 * Runs for 4 seconds for each test iteration.
 */
#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static void run(void)
{
	int runtime;

	tst_res(TINFO, "Running variant %i", tst_variant);

	do {
		runtime = tst_remaining_runtime();
		tst_res(TINFO, "Remaining runtime %d", runtime);
		sleep(1);
	} while (runtime);

	tst_res(TPASS, "Finished loop!");
}

static struct tst_test test = {
	.test_all = run,
	.runtime = 4,
	.test_variants = 2,
};

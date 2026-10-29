// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Test: checkpoint wake without matching wait.
 * Expected: wake completes with ETIMEDOUT errno as expected.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tst_test.h"
#include "tst_checkpoint.h"

static void run(void)
{
	int ret = tst_checkpoint_wake(0, 1, 1000);

	if (ret == -1 && errno == ETIMEDOUT)
		tst_res(TPASS, "checkpoint wake timed out as expected");
	else
		tst_brk(TBROK | TERRNO, "checkpoint wake failed");

	return;
}

static struct tst_test test = {
	.test_all = run,
	.needs_checkpoints = 1,
};

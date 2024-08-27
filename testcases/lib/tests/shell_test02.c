// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Shell test example.
 */

#include "tst_test.h"

static void run_test(void)
{
	tst_run_script("shell_test_pass.sh", NULL);
	tst_reap_children();
	tst_res(TINFO, "Shell test has finished at this point!");
}

static struct tst_test test = {
	.runs_script = 1,
	.test_all = run_test,
};

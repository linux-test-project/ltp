// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#include "tst_test.h"

static void do_test(void)
{
	switch (tst_variant) {
	case 0:
		/* This is skipped after first iteration */
		tst_brk(TCONF, "Test skipped");
	break;
	case 1:
		/* This test is correctly looped with -i opt */
		tst_res(TPASS, "Test passed");
	break;
	case 2:
		/* This exits the test immediatelly */
		tst_brk(TBROK, "Test broken");
	break;
	}

	tst_res(TINFO, "test() function exitting normaly");
}

static void setup(void)
{
	tst_res(TINFO, "Running test setup()");

	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Starting tst_brk(TCONF) test");
	break;
	case 1:
		tst_res(TINFO, "Starting tst_res(TPASS) test");
	break;
	case 2:
		tst_res(TINFO, "Starting tst_brk(TBROK) test");
	break;
	}
}

static void cleanup(void)
{
	tst_res(TINFO, "Running test cleanup()");
}

static struct tst_test test = {
	.test_all = do_test,
	.test_variants = 3,
	.setup = setup,
	.cleanup = cleanup,
};

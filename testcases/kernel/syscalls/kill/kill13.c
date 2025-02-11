// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * Reproducer of CVE-2018-10124; INT_MIN negation.
 *
 * On most two's complement CPUs negation of INT_MIN will result in
 * INT_MIN because ~((unsigned)INT_MIN) + 1 overflows to INT_MIN
 * (unless trapped). On one's complement ~((unsigned)INT_MIN) = INT_MAX.
 *
 * Without UBSAN kill will always return ESRCH. Regardless of if the
 * bug is present as INT_MIN/INT_MAX are invalid PIDs. It checks the
 * PID before the signal number so we can not cause EINVAL. A trivial
 * test of kill is performed elsewhere. So we don't run the test
 * without UBSAN to avoid giving the impression we have actually
 * tested for the bug.
 */

#include <limits.h>
#include <signal.h>
#include "tst_test.h"

static void run(void)
{
	TST_EXP_FAIL2(kill(INT_MIN, 0), ESRCH,
		      "kill(INT_MIN, ...) fails with ESRCH");
}

static struct tst_test test = {
	.test_all = run,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_UBSAN_SIGNED_OVERFLOW",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "4ea77014af0d"},
		{"CVE", "CVE-2018-10124"},
		{}
	}
};

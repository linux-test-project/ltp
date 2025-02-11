// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * Check wait4(INT_MIN, ...) is not allowed. The pid is negated before
 * searching for a group with that pid. Negating INT_MIN is not
 * defined so UBSAN will be triggered if enabled. Also see kill13.
 *
 * If the bug is present, but UBSAN is not enabled, then it should
 * result in ECHILD.
 */

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "tst_test.h"

static void run(void)
{
	int status = 1;
	struct rusage rusage;

	TST_EXP_FAIL2(wait4(INT_MIN, &status, 0, &rusage), ESRCH,
		      "wait4 fails with ESRCH");
}

static struct tst_test test = {
	.test_all = run,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "dd83c161fbcc"},
		{}
	}
};

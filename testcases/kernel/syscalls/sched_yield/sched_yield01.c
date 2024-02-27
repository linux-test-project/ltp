// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *
 * Testcase to check that sched_yield returns correct values.
 */

#include <sched.h>
#include "tst_test.h"

static void run(void)
{
	TST_EXP_PASS(sched_yield());
}

static struct tst_test test = {
	.test_all = run,
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:sysinfo(2) returns EFAULT for an invalid address
 * structure.
 */

#include <sys/sysinfo.h>
#include "tst_test.h"

static struct sysinfo *bad_info;

static void setup(void)
{
	bad_info = tst_get_bad_addr(NULL);
}

static void run(void)
{
	TST_EXP_FAIL(sysinfo(bad_info), EFAULT);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};

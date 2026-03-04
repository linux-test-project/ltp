// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2024
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Tests the basic functionality of :manpage:`ulimit(3)` with UL_GETFSIZE and
 * UL_SETFSIZE.
 */

#include <ulimit.h>
#include "tst_test.h"

static void run(void)
{
	long current_fsize;

	TST_EXP_POSITIVE(ulimit(UL_GETFSIZE, -1));

	if (!TST_PASS)
		return;

	current_fsize = TST_RET;

	TST_EXP_POSITIVE(ulimit(UL_SETFSIZE, current_fsize),
			 "ulimit(UL_SETFSIZE, %ld) (same value)", current_fsize);

	TST_EXP_POSITIVE(ulimit(UL_SETFSIZE, current_fsize - 1),
			 "ulimit(UL_SETFSIZE, %ld) (value - 1)", current_fsize - 1);
}

static struct tst_test test = {
	.test_all = run,
};

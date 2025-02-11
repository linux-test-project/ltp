// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Call close(-1) and expects it to return EBADF.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "tst_test.h"

static void run(void)
{
	TST_EXP_FAIL(close(-1), EBADF);
}

static struct tst_test test = {
	.test_all = run,
};

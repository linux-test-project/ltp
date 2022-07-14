// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that open(2) fails with ENXIO when
 * O_NONBLOCK | O_WRONLY is set, the named file is a FIFO,
 * and no process has the FIFO open for reading.
 */

#include "tst_test.h"

#define TEMP_FIFO "tmpfile"

static void setup(void)
{
	SAFE_MKFIFO(TEMP_FIFO, 0644);
}

static void run(void)
{
	TST_EXP_FAIL2(open(TEMP_FIFO, O_NONBLOCK | O_WRONLY), ENXIO);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1
};

// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * 05/2002 Ported by André Merlier
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * Verify that mknod() fails with -1 and sets errno to EINVAL if the mode is
 * different than a normal file, device special file or FIFO.
 */

#include "tst_test.h"

static void check_mknod(void)
{
	TST_EXP_FAIL(mknod("tnode", S_IFMT, 0), EINVAL);
}

static struct tst_test test = {
	.test_all = check_mknod,
	.needs_tmpdir = 1,
	.needs_root = 1
};

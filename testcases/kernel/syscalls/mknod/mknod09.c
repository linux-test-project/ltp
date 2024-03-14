// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * HISTORY
 *	05/2002 Ported by André Merlier
 *
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *  Verify that, mknod() fails with -1 and sets errno to EINVAL if the mode is
 *  different than a normal file, device special file or FIFO.
 *
 * Expected Result:
 *  mknod() should fail with return value -1 and sets expected errno.
 *
 * RESTRICTIONS:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include "tst_test.h"

#define MODE_RWX	S_IFMT	/* mode different from those expected */

static void check_mknod(void)
{
	/*
	 * Call mknod(2) to test condition.
	 * verify that it fails with -1 return value and
	 * sets appropriate errno.
	 */
	TST_EXP_FAIL(mknod("tnode", MODE_RWX, 0), EINVAL);
}

static struct tst_test test = {
	.test_all = check_mknod,
	.needs_tmpdir = 1,
	.needs_root = 1
};

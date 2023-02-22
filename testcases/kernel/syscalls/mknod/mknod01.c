// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR: William Roske, CO-PILOT: Dave Fenner
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that mknod(2) successfully creates a filesystem node with
 * various modes.
 */

#include <sys/sysmacros.h>
#include "tst_test.h"

#define PATH "test_node"

static int tcases[] = {
	S_IFREG | 0777,
	S_IFIFO | 0777,
	S_IFCHR | 0777,
	S_IFBLK | 0777,

	S_IFREG | 04700,
	S_IFREG | 02700,
	S_IFREG | 06700,
};


static void run(unsigned int i)
{
	dev_t dev = 0;

	if (S_ISCHR(tcases[i]) || S_ISBLK(tcases[i]))
		dev = makedev(1, 3);

	TST_EXP_PASS(mknod(PATH, tcases[i], dev),
				"mknod(PATH, %o, %ld)",
				tcases[i], dev);
	SAFE_UNLINK(PATH);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that mknod(2) fails with the correct error codes:
 *
 * - ENAMETOOLONG if the pathname component was too long.
 * - EEXIST if specified path already exists.
 * - EFAULT if pathname points outside user's accessible address space.
 * - ENOENT if the directory component in pathname does not exist.
 * - ENOENT if the pathname is empty.
 * - ENOTDIR if the directory component in pathname is not a directory.
 */

#include "tst_test.h"

#define MODE_FIFO_RWX (S_IFIFO | 0777)

static char *longpathname;

static struct tcase {
	char *pathname;
	char *desc;
	int exp_errno;
} tcases[] = {
	{ NULL, "Pathname too long", ENAMETOOLONG },
	{ "tnode_1", "Specified node already exists", EEXIST },
	{ NULL, "Invalid address", EFAULT },
	{ "testdir_2/tnode_2", "Non-existent file", ENOENT },
	{ "", "Pathname is empty", ENOENT },
	{ "tnode/tnode_3", "Path contains regular file", ENOTDIR },
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(mknod(tc->pathname, MODE_FIFO_RWX, 0), tc->exp_errno, "%s",
		     tc->desc);
}

static void setup(void)
{
	SAFE_MKNOD("tnode_1", MODE_FIFO_RWX, 0);
	SAFE_MKNOD("tnode", MODE_FIFO_RWX, 0);

	for (int i = 0; i <= (PATH_MAX + 1); i++)
		longpathname[i] = 'a';
	tcases[0].pathname = longpathname;
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers[]) {
		{ &longpathname, .size = PATH_MAX + 2 },
		{},
	},
};

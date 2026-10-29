// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2000-2023
 * Authors: William Roske, Dave Fenner
 */

/*\
 * Check the basic functionality of the pathconf(2) system call.
 */

#include <stdlib.h>
#include "tst_test.h"

#define NAME_DESC(x) .value = x, .name = #x

static char *path;

static struct tcase {
	int value;
	char *name;
	/* POSIX leaves support for a directory unspecified */
	int einval_ok;
} tcases[] = {
	{NAME_DESC(_PC_LINK_MAX)},
	{NAME_DESC(_PC_MAX_CANON), .einval_ok = 1},
	{NAME_DESC(_PC_MAX_INPUT), .einval_ok = 1},
	{NAME_DESC(_PC_NAME_MAX)},
	{NAME_DESC(_PC_PATH_MAX)},
	{NAME_DESC(_PC_PIPE_BUF)},
	{NAME_DESC(_PC_CHOWN_RESTRICTED)},
	{NAME_DESC(_PC_NO_TRUNC)},
	{NAME_DESC(_PC_VDISABLE), .einval_ok = 1},
	{NAME_DESC(_PC_SYNC_IO), .einval_ok = 1},
	{NAME_DESC(_PC_ASYNC_IO), .einval_ok = 1},
	{NAME_DESC(_PC_PRIO_IO), .einval_ok = 1},
	{NAME_DESC(_PC_FILESIZEBITS)},
	{NAME_DESC(_PC_REC_INCR_XFER_SIZE), .einval_ok = 1},
	{NAME_DESC(_PC_REC_MAX_XFER_SIZE), .einval_ok = 1},
	{NAME_DESC(_PC_REC_MIN_XFER_SIZE), .einval_ok = 1},
	{NAME_DESC(_PC_REC_XFER_ALIGN), .einval_ok = 1},
};

static void verify_pathconf(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	path = tst_tmpdir_path();

	TEST(pathconf(path, tc->value));

	if (TST_RET != -1 || !TST_ERR) {
		tst_res(TPASS, "pathconf(%s, %s)", path, tc->name);
		return;
	}

	if (tc->einval_ok && TST_ERR == EINVAL) {
		tst_res(TCONF, "pathconf(%s, %s) not supported for a directory",
			path, tc->name);
		return;
	}

	tst_res(TFAIL | TTERRNO, "pathconf(%s, %s)", path, tc->name);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = verify_pathconf,
	.tcnt = ARRAY_SIZE(tcases),
};

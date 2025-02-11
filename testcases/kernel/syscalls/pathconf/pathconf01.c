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
} tcases[] = {
	{NAME_DESC(_PC_LINK_MAX)},
	{NAME_DESC(_PC_MAX_CANON)},
	{NAME_DESC(_PC_MAX_INPUT)},
	{NAME_DESC(_PC_NAME_MAX)},
	{NAME_DESC(_PC_PATH_MAX)},
	{NAME_DESC(_PC_PIPE_BUF)},
	{NAME_DESC(_PC_CHOWN_RESTRICTED)},
	{NAME_DESC(_PC_NO_TRUNC)},
	{NAME_DESC(_PC_VDISABLE)},
	{NAME_DESC(_PC_SYNC_IO)},
	{NAME_DESC(_PC_ASYNC_IO)},
	{NAME_DESC(_PC_PRIO_IO)},
	{NAME_DESC(_PC_FILESIZEBITS)},
	{NAME_DESC(_PC_REC_INCR_XFER_SIZE)},
	{NAME_DESC(_PC_REC_MAX_XFER_SIZE)},
	{NAME_DESC(_PC_REC_MIN_XFER_SIZE)},
	{NAME_DESC(_PC_REC_XFER_ALIGN)},
};

static void verify_pathconf(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	path = tst_tmpdir_path();

	TEST(pathconf(path, tc->value));

	if (TST_RET == -1 && errno != 0)
		tst_res(TFAIL, "pathconf Failed, errno = %d", TST_ERR);
	else
		tst_res(TPASS, "pathconf(%s, %s)", path, tc->name);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = verify_pathconf,
	.tcnt = ARRAY_SIZE(tcases),
};

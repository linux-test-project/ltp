// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * 06/1994 AUTHOR: Richard Logan CO-PILOT: William Roske
 * Copyright (c) 2023 SUSE LLC
 */

/*\
 * [Description]
 *
 * Basic test for dup(2) of a system pipe descriptor.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "tst_test.h"

static int fd[2];

static void run(void)
{
	TST_EXP_FD(dup(fd[0]), "dup(%d) read end of the pipe", fd[0]);
	SAFE_CLOSE(TST_RET);

	TST_EXP_FD(dup(fd[1]), "dup(%d) write end of the pipe", fd[1]);
	SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	fd[0] = -1;

	SAFE_PIPE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
};

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * 06/1994 AUTHOR: Richard Logan CO-PILOT: William Roske
 */

/*\
 * [DESCRIPTION]
 *
 * Basic test for dup(2) of a system pipe descriptor.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "tst_test.h"

int fd[2];

static void run(void)
{
	TEST(dup(fd[0]));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO,
			 "dup of read side of pipe failed");
	else {
		tst_res(TPASS,
			 "dup(%d) read side of syspipe returned %ld",
			 fd[0], TST_RET);

		SAFE_CLOSE(TST_RET);
	}

	TEST(dup(fd[1]));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO,
			 "dup of write side of pipe failed");
	} else {
		tst_res(TPASS,
			 "dup(%d) write side of syspipe returned %ld",
			 fd[1], TST_RET);

		SAFE_CLOSE(TST_RET);
	}
}

void setup(void)
{
	fd[0] = -1;

	SAFE_PIPE(fd);
}

static struct tst_test test = {
        .test_all = run,
        .setup = setup,
        .needs_tmpdir = 1,
};

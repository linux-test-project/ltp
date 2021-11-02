// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * 06/1994 AUTHOR: Richard Logan CO-PILOT: William Roske
 */

/*\
 * [DESCRIPTION]
 *
 * Basic test for dup(2) of a named pipe descriptor
 */
#include <stdio.h>
#include "tst_test.h"

char Fname[255];
int fd;

static void run(void)
{
	TEST(dup(fd));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "dup failed");
	} else {
		tst_res(TPASS, "dup returned %ld",
			 TST_RET);

		SAFE_CLOSE(TST_RET);
	}
}

void setup(void)
{
	fd = -1;

	sprintf(Fname, "dupfile");
	SAFE_MKFIFO(Fname, 0777);
	if ((fd = open(Fname, O_RDWR, 0700)) == -1)
		tst_brk(TBROK, "open failed");
}

void cleanup(void)
{
	if (fd != -1)
		if (close(fd) == -1)
			tst_res(TWARN | TERRNO, "close failed");
}

static struct tst_test test = {
        .test_all = run,
        .setup = setup,
        .cleanup = cleanup,
	.needs_tmpdir = 1,
};

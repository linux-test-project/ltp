// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Red Hat Inc., 2007
 * 12/2007 Copyed from sendfile03.c by Masatake YAMATO
 */

/*\
 * Testcase to test that sendfile(2) system call returns EAGAIN
 * when passing full out_fd opened with O_NONBLOCK.
 */

#include <sys/sendfile.h>
#include "tst_test.h"

#define MAX_FILL_DATA_LENGTH 0xFFFFFFF

static int p[2];
static int in_fd;
static int out_fd;

static void setup(void)
{
	int i;

	tst_fill_file("in_file", 'a', 10, 1);
	in_fd = SAFE_OPEN("in_file", O_RDONLY);

	SAFE_SOCKETPAIR(PF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0, p);
	out_fd = p[1];

	for (i = 0; i < MAX_FILL_DATA_LENGTH; ++i) {
		TEST(write(out_fd, "a", 1));
		if (TST_RET < 0) {
			if (TST_ERR == EAGAIN)
				return;
			else
				tst_brk(TBROK | TTERRNO, "write(out_fd, buf, 1)");
		}
	}

	tst_brk(TBROK, "Failed to get EAGAIN after %i bytes",
	        MAX_FILL_DATA_LENGTH);
}

static void cleanup(void)
{
	if (p[0])
		SAFE_CLOSE(p[0]);
	if (p[1])
		SAFE_CLOSE(p[1]);
	SAFE_CLOSE(in_fd);
}

static void run(void)
{
	TST_EXP_FAIL(sendfile(out_fd, in_fd, NULL, 1), EAGAIN,
		     "sendfile(out_fd, in_fd, NULL, 1) with blocked out_fd");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
	.test_all = run,
};

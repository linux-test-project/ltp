// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Testcase to test that sendfile(2) system call returns EBADF when passing
 * wrong out_fd or in_fd.
 *
 * There are four cases:
 *
 * - in_fd == -1
 * - out_fd = -1
 * - in_fd opened with O_WRONLY
 * - out_fd opened with O_RDONLY
 */

#include <sys/sendfile.h>
#include "tst_test.h"

static int in_fd;
static int out_fd;
static int negative_fd = -1;

struct test_case_t {
	int *in_fd;
	int *out_fd;
	const char *desc;
} tc[] = {
	{&in_fd, &negative_fd, "out_fd=-1"},
	{&in_fd, &in_fd, "out_fd=O_RDONLY"},
	{&negative_fd, &out_fd, "in_fd=-1"},
	{&out_fd, &out_fd, "out_fd=O_WRONLY"},
};

static void setup(void)
{
	in_fd = SAFE_OPEN("in_file", O_CREAT | O_RDONLY, 0600);
	out_fd = SAFE_CREAT("out_file", 0600);
}

static void cleanup(void)
{
	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static void run(unsigned int i)
{
	TST_EXP_FAIL2(sendfile(*(tc[i].out_fd), *(tc[i].in_fd), NULL, 1),
		     EBADF, "sendfile(..) with %s", tc[i].desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
	.test = run,
};

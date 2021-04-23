// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Red Hat Inc., 2007
 * 11/2007 Copyed from sendfile02.c by Masatake YAMATO
 */

/*\
 * [Description]
 *
 * Testcase to test that sendfile(2) system call returns EINVAL when passing
 * negative offset.
 *
 * [Algorithm]
 *
 * Call sendfile with offset = -1.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tst_test.h"

static int in_fd;
static int out_fd;

static void setup(void)
{
	in_fd = SAFE_OPEN("in_file", O_CREAT | O_RDWR, 0600);
	out_fd = SAFE_CREAT("out_file", 0600);
}

static void cleanup(void)
{
	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static void run(void)
{
	off_t offset = -1;

	TST_EXP_FAIL(sendfile(out_fd, in_fd, &offset, 1), EINVAL,
		     "sendfile(out, in, &offset, ..) with offset=%ld", offset);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
	.test_all = run,
};

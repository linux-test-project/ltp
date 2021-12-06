// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2006
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/poll.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/splice.h"
#include "lapi/vmsplice.h"

#define TEST_BLOCK_SIZE (1<<17)	/* 128K */

#define TESTFILE "vmsplice_test_file"

static int fd_out;
static char buffer[TEST_BLOCK_SIZE];

static void check_file(void)
{
	int i;
	char vmsplicebuffer[TEST_BLOCK_SIZE];

	fd_out = SAFE_OPEN(TESTFILE, O_RDONLY);
	SAFE_READ(1, fd_out, vmsplicebuffer, TEST_BLOCK_SIZE);

	for (i = 0; i < TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != vmsplicebuffer[i])
			break;
	}

	if (i < TEST_BLOCK_SIZE)
		tst_res(TFAIL, "Wrong data read from the buffer at %i", i);
	else
		tst_res(TPASS, "Written data has been read back correctly");

	SAFE_CLOSE(fd_out);
}

static void vmsplice_test(void)
{
	int pipes[2];
	long written;
	int ret;
	int fd_out;
	struct iovec v;
	loff_t offset;

	v.iov_base = buffer;
	v.iov_len = TEST_BLOCK_SIZE;

	fd_out = SAFE_OPEN(TESTFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	SAFE_PIPE(pipes);

	struct pollfd pfd = {.fd = pipes[1], .events = POLLOUT};
	offset = 0;

	while (v.iov_len) {
		/*
		 * in a real app you'd be more clever with poll of course,
		 * here we are basically just blocking on output room and
		 * not using the free time for anything interesting.
		 */
		if (poll(&pfd, 1, -1) < 0)
			tst_brk(TBROK | TERRNO, "poll() failed");

		written = vmsplice(pipes[1], &v, 1, 0);
		if (written < 0) {
			tst_brk(TBROK | TERRNO, "vmsplice() failed");
		} else {
			if (written == 0) {
				break;
			} else {
				v.iov_base += written;
				v.iov_len -= written;
			}
		}

		ret = splice(pipes[0], NULL, fd_out, &offset, written, 0);
		if (ret < 0)
			tst_brk(TBROK | TERRNO, "splice() failed");
		//printf("offset = %lld\n", (long long)offset);
	}

	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);
	SAFE_CLOSE(fd_out);

	check_file();
}

static void setup(void)
{
	int i;

	for (i = 0; i < TEST_BLOCK_SIZE; i++)
		buffer[i] = i & 0xff;
}

static void cleanup(void)
{
	if (fd_out > 0)
		SAFE_CLOSE(fd_out);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = vmsplice_test,
	.needs_tmpdir = 1,
	.skip_filesystems = (const char *const []) {
		"nfs",
		NULL
	},
	.min_kver = "2.6.17",
};

/******************************************************************************
 *
 * Copyright (c) International Business Machines  Corp., 2006
 *   Author Yi Yang <yyangcdl@cn.ibm.com>
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * DESCRIPTION
 *	This test case will verify basic function of vmsplice
 *	added by kernel 2.6.17 or up.
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
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

	if (tst_fs_type(".") == TST_NFS_MAGIC) {
		tst_brk(TCONF, "Cannot do splice() "
			 "on a file located on an NFS filesystem");
	}

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
	.min_kver = "2.6.17",
};

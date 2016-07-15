/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * DESCRIPTION
 *	test 1:
 *	Read with an invalid file descriptor, and expect an EBADF.
 *
 *	test 2:
 *	The parameter passed to read is a directory, check if the errno is
 *	set to EISDIR.
 *
 *	test 3:
 *	Buf is outside the accessible address space, expect an EFAULT.
 *
 *	test 4:
 *	The file was opened with the O_DIRECT flag, and transfer sizes was not
 *	multiples of the logical block size of the file system, expect an
 *	EINVAL.
 *
 *	test 5:
 *	The file was opened with the O_DIRECT flag, and the alignment of the
 *	user buffer was not multiples of the logical block size of the file
 *	system, expect an EINVAL.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "read02";

static int badfd = -1;
static int fd2, fd3, fd4 = -1;
static char buf[BUFSIZ];
static void *bufaddr = buf;
static void *outside_buf = (void *)-1;
static void *addr4;
static void *addr5;

static long fs_type;

static struct test_case_t {
	int *fd;
	void **buf;
	size_t count;
	int exp_error;
} TC[] = {
	{&badfd, &bufaddr, 1, EBADF},
	{&fd2, &bufaddr, 1, EISDIR},
#ifndef UCLINUX
	{&fd3, &outside_buf, 1, EFAULT},
#endif
	{&fd4, &addr4, 1, EINVAL},
	{&fd4, &addr5, 4096, EINVAL},
};

int TST_TOTAL = ARRAY_SIZE(TC);
static void setup(void);
static void cleanup(void);
static void read_verify(const struct test_case_t *);

int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			read_verify(&TC[i]);
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd2 = SAFE_OPEN(cleanup, ".", O_DIRECTORY);

	SAFE_FILE_PRINTF(cleanup, "test_file", "A");

	fd3 = SAFE_OPEN(cleanup, "test_file", O_RDWR);

#if !defined(UCLINUX)
	outside_buf = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
				MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
#endif

	addr4 = SAFE_MEMALIGN(cleanup, getpagesize(), (4096 * 10));
	addr5 = addr4 + 1;

	fs_type = tst_fs_type(cleanup, ".");
	if (fs_type != TST_TMPFS_MAGIC)
		fd4 = SAFE_OPEN(cleanup, "test_file", O_RDWR | O_DIRECT);
}

static void read_verify(const struct test_case_t *test)
{
	if (test->fd == &fd4 && *test->fd == -1) {
		tst_resm(TCONF, "O_DIRECT not supported on %s filesystem",
		         tst_fs_type_name(fs_type));
		return;
	}

	TEST(read(*test->fd, *test->buf, test->count));

	if (*test->fd == fd4 && TEST_RETURN >= 0) {
		tst_resm(TPASS,
			 "O_DIRECT unaligned reads fallbacks to buffered I/O");
		return;
	}

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_error) {
		tst_resm(TPASS | TTERRNO, "expected failure");
	} else {
		tst_resm(TFAIL | TTERRNO, "unexpected error expected %d",
			 test->exp_error);
	}
}

static void cleanup(void)
{
	free(addr4);

	if (fd4 > 0)
		close(fd4);

	if (fd3 > 0)
		close(fd3);

	if (fd2 > 0)
		close(fd2);

	tst_rmdir();
}

/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*
 * errno tests for readahead() syscall
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "config.h"
#include "test.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

char *TCID = "readahead01";
int TST_TOTAL = 1;

option_t options[] = {
	{NULL, NULL, NULL}
};

#if defined(__NR_readahead)
static void setup(void);
static void cleanup(void);

static int check_ret(long expected_ret)
{
	if (expected_ret == TEST_RETURN) {
		tst_resm(TPASS, "expected ret success - "
			 "returned value = %ld", TEST_RETURN);
		return 0;
	}
	tst_resm(TFAIL, "unexpected failure - "
		 "returned value = %ld, expected: %ld",
		 TEST_RETURN, expected_ret);
	return 1;
}

static int check_errno(long expected_errno)
{
	if (TEST_ERRNO == expected_errno) {
		tst_resm(TPASS | TTERRNO, "expected failure");
		return 0;
	}

	if (TEST_ERRNO == 0)
		tst_resm(TFAIL, "call succeeded unexpectedly");
	else
		tst_resm(TFAIL | TTERRNO, "unexpected failure - "
			 "expected = %ld : %s, actual",
			 expected_errno, strerror(expected_errno));
	return 1;
}

static void test_bad_fd(void)
{
	char tempname[PATH_MAX] = "readahead01_XXXXXX";
	int fd;

	tst_resm(TINFO, "test_bad_fd -1");
	TEST(readahead(-1, 0, getpagesize()));
	check_ret(-1);
	check_errno(EBADF);

	tst_resm(TINFO, "test_bad_fd O_WRONLY");
	fd = mkstemp(tempname);
	if (fd == -1)
		tst_resm(TBROK | TERRNO, "mkstemp failed");
	close(fd);
	fd = open(tempname, O_WRONLY);
	if (fd == -1)
		tst_resm(TBROK | TERRNO, "Failed to open testfile");
	TEST(readahead(fd, 0, getpagesize()));
	check_ret(-1);
	check_errno(EBADF);
	close(fd);
	unlink(tempname);
}

static void test_invalid_fd(void)
{
	int fd[2];

	tst_resm(TINFO, "test_invalid_fd pipe");
	if (pipe(fd) < 0)
		tst_resm(TBROK | TERRNO, "Failed to create pipe");
	TEST(readahead(fd[0], 0, getpagesize()));
	check_ret(-1);
	check_errno(EINVAL);
	close(fd[0]);
	close(fd[1]);

	tst_resm(TINFO, "test_invalid_fd socket");
	fd[0] = socket(AF_INET, SOCK_STREAM, 0);
	if (fd[0] < 0)
		tst_resm(TBROK | TERRNO, "Failed to create socket");
	TEST(readahead(fd[0], 0, getpagesize()));
	check_ret(-1);
	check_errno(EINVAL);
	close(fd[0]);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_bad_fd();
		test_invalid_fd();
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	tst_tmpdir();

	/* check if readahead syscall is supported */
	ltp_syscall(__NR_readahead, 0, 0, 0);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}

#else /* __NR_readahead */
int main(void)
{
	tst_brkm(TCONF, NULL, "System doesn't support __NR_readahead");
}
#endif

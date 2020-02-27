// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *  05/2019 Ported to new library: Christian Amann <camann@suse.com>
 */
/*
 * Tests different error scenarios:
 *
 * 1) Calls fstat() with closed file descriptor
 *    -> EBADF
 * 2) Calls fstat() with an invalid address for stat structure
 *    -> EFAULT (or receive signal SIGSEGV)
 */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#define TESTFILE	"test_file"

static int fd_ok;
static int fd_ebadf = -1;
static struct stat stat_buf;

static struct tcase {
	int *fd;
	struct stat *stat_buf;
	int exp_err;
} tcases[] = {
	{&fd_ebadf, &stat_buf, EBADF},
	{&fd_ok, NULL, EFAULT},
};

static void check_fstat(unsigned int tc_num)
{
	struct tcase *tc = &tcases[tc_num];

	TEST(fstat(*tc->fd, tc->stat_buf));
	if (TST_RET == -1) {
		if (tc->exp_err == TST_ERR) {
			tst_res(TPASS,
				 "fstat() fails with expected error %s",
				 tst_strerrno(tc->exp_err));
		} else {
			tst_res(TFAIL | TTERRNO,
				 "fstat() did not fail with %s, but with",
				 tst_strerrno(tc->exp_err));
		}
	} else {
		tst_res(TFAIL, "fstat() returned %ld, expected -1",
			 TST_RET);
	}
}

static void run(unsigned int tc_num)
{
	pid_t pid;
	int status;

	pid = SAFE_FORK();
	if (pid == 0) {
		check_fstat(tc_num);
		return;
	}
	SAFE_WAITPID(pid, &status, 0);

	if (tcases[tc_num].exp_err == EFAULT && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "fstat() failed as expected with SIGSEGV");
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	tst_res(TFAIL, "child %s", tst_strstatus(status));
}

static void setup(void)
{
	fd_ok = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);
}

static void cleanup(void)
{
	if (fd_ok > 0)
		SAFE_CLOSE(fd_ok);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};

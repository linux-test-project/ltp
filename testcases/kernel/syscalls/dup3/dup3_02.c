/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 * Verify that,
 *  1. dup3() fails with -1 return value and sets errno to EINVAL
 *     if flags contain an invalid value or oldfd was equal to newfd.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"


static void setup(void);
static void cleanup(void);

#define INVALID_FLAG	-1

static int old_fd;
static int new_fd;

static struct test_case_t {
	int *oldfd;
	int *newfd;
	int flags;
	int exp_errno;
} test_cases[] = {
	{&old_fd, &old_fd, O_CLOEXEC, EINVAL},
	{&old_fd, &old_fd, 0, EINVAL},
	{&old_fd, &new_fd, INVALID_FLAG, EINVAL}
};

char *TCID = "dup3_02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ltp_syscall(__NR_dup3, *(test_cases[i].oldfd),
			     *(test_cases[i].newfd), test_cases[i].flags));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "dup3 succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == test_cases[i].exp_errno) {
				tst_resm(TPASS | TTERRNO,
					 "dup3 failed as expected");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "dup3 failed unexpectedly; expected:"
					 "%d - %s", test_cases[i].exp_errno,
					 strerror(test_cases[i].exp_errno));
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	old_fd = SAFE_CREAT(cleanup, "testeinval.file", 0644);
	new_fd = -1;
}

static void cleanup(void)
{
	if (old_fd > 0)
		SAFE_CLOSE(NULL, old_fd);

	tst_rmdir();
}

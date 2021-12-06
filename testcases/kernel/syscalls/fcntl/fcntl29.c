/*
 * Copyright (c) 2014 Fujitsu Ltd.
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
 *   Basic test for fcntl(2) using F_DUPFD_CLOEXEC argument.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

char *TCID = "fcntl29";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

static int test_fd;

int main(int ac, char **av)
{
	int lc, dup_fd;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(fcntl(test_fd, F_DUPFD_CLOEXEC, 0));
		if (TEST_RETURN < 0) {
			tst_brkm(TFAIL | TTERRNO, cleanup, "fcntl "
				 "test F_DUPFD_CLOEXEC failed");
		}
		dup_fd = TEST_RETURN;

		TEST(fcntl(dup_fd, F_GETFD));
		if (TEST_RETURN < 0) {
			SAFE_CLOSE(cleanup, dup_fd);
			tst_brkm(TFAIL | TTERRNO, cleanup, "fcntl "
				 "test F_GETFD failed");
		}

		if (TEST_RETURN & FD_CLOEXEC) {
			tst_resm(TPASS, "fcntl test "
				 "F_DUPFD_CLOEXEC success");
		} else {
			tst_resm(TFAIL, "fcntl test "
				 "F_DUPFD_CLOEXEC fail");
		}

		SAFE_CLOSE(cleanup, dup_fd);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 24)) < 0) {
		tst_brkm(TCONF, NULL, "Kernels >= 2.6.24 required");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	test_fd = SAFE_CREAT(cleanup, "testfile", 0644);
}

static void cleanup(void)
{
	if (test_fd > 0)
		SAFE_CLOSE(NULL, test_fd);

	tst_rmdir();
}

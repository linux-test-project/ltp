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
 *     Basic test for fcntl(2) using F_SETPIPE_SZ, F_GETPIPE_SZ argument.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

char *TCID = "fcntl30";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int pipe_fds[2], test_fd;
	int orig_pipe_size, new_pipe_size;


	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_PIPE(cleanup, pipe_fds);
		test_fd = pipe_fds[1];

		TEST(fcntl(test_fd, F_GETPIPE_SZ));
		if (TEST_RETURN < 0) {
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "fcntl get pipe size failed");
		}

		orig_pipe_size = TEST_RETURN;
		new_pipe_size = orig_pipe_size * 2;
		TEST(fcntl(test_fd, F_SETPIPE_SZ, new_pipe_size));
		if (TEST_RETURN < 0) {
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "fcntl test F_SETPIPE_SZ failed");
		}

		TEST(fcntl(test_fd, F_GETPIPE_SZ));
		if (TEST_RETURN < 0) {
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "fcntl test F_GETPIPE_SZ failed");
		}
		tst_resm(TINFO, "orig_pipe_size: %d new_pipe_size: %d",
			 orig_pipe_size, new_pipe_size);
		if (TEST_RETURN >= new_pipe_size) {
			tst_resm(TPASS, "fcntl test F_GETPIPE_SZ and F_SETPIPE_SZ passed");
		} else {
			tst_resm(TFAIL, "fcntl test F_GETPIPE_SZ and F_SETPIPE_SZ failed");
		}
		SAFE_CLOSE(cleanup, pipe_fds[0]);
		SAFE_CLOSE(cleanup, pipe_fds[1]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 35)) < 0) {
		tst_brkm(TCONF, NULL, "kernel >= 2.6.35 required");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

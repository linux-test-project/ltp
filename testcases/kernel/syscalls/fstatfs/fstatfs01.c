/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/*
 * DETAILED DESCRIPTION
 *   This is a Phase I test for the fstatfs(2) system call.  It is intended
 *   to provide a limited exposure of the system call, for now.  It
 *   should/will be extended when full functional tests are written for
 *   fstatfs(2).
 */

#include <sys/types.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "fstatfs01";

static int file_fd;
static int pipe_fd;

static struct tcase {
	int *fd;
	const char *msg;
} tcases[2] = {
	{&file_fd, "fstatfs() on a file"},
	{&pipe_fd, "fstatfs() on a pipe"},
};

int TST_TOTAL = ARRAY_SIZE(tcases);

int main(int ac, char **av)
{
	int lc, i;
	struct statfs stats;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(fstatfs(*tcases[i].fd, &stats));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO, "%s", tcases[i].msg);
			} else {
				tst_resm(TPASS, "%s - f_type=%lx",
				         tcases[i].msg, stats.f_type);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int pipe[2];

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	file_fd = SAFE_OPEN(cleanup, "test_file", O_RDWR | O_CREAT, 0700);

	SAFE_PIPE(cleanup, pipe);
	pipe_fd = pipe[0];
	SAFE_CLOSE(cleanup, pipe[1]);
}

static void cleanup(void)
{
	if (file_fd > 0 && close(file_fd))
		tst_resm(TWARN | TERRNO, "close(file_fd) failed");

	if (pipe_fd > 0 && close(pipe_fd))
		tst_resm(TWARN | TERRNO, "close(pipe_fd) failed");

	tst_rmdir();
}

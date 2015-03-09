/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Testcase to check open(2) sets errno to ENXIO correctly.
 *
 * ALGORITHM
 *	Create a named pipe using mknod(2).  Attempt to
 *	open(2) the pipe for writing. The open(2) should
 *	fail with ENXIO.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"

char *TCID = "open06";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

static char fname[100] = "fifo";

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(open(fname, O_NONBLOCK | O_WRONLY));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "open(2) succeeded unexpectedly");
			continue;
		}

		if (TEST_ERRNO != ENXIO)
			tst_resm(TFAIL, "Expected ENXIO got %d", TEST_ERRNO);
		else
			tst_resm(TPASS, "call returned expected ENXIO error");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "%s.%d", fname, getpid());

	if (mknod(fname, S_IFIFO | 0644, 0) == -1)
		tst_brkm(TBROK, cleanup, "mknod FAILED");
}

static void cleanup(void)
{
	unlink(fname);

	tst_rmdir();
}

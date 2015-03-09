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
 * NAME
 *	open01.c
 *
 * DESCRIPTION
 *	Open a file with oflag = O_CREAT set, does it set the sticky bit off?
 *
 *	Open "/tmp" with O_DIRECTORY, does it set the S_IFDIR bit on?
 *
 * ALGORITHM
 *	1. open a new file with O_CREAT, fstat.st_mode should not have the
 *	   01000 bit on. In Linux, the save text bit is *NOT* cleared.
 *
 *	2. open "/tmp" with O_DIRECTORY.  fstat.st_mode should have the
 *	   040000 bit on.
 *
 * USAGE:  <for command-line>
 *  open01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#define _GNU_SOURCE		/* for O_DIRECTORY */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "test.h"

char *TCID = "open01";
int TST_TOTAL = 1;

static char pfilname[40] = "";

static void cleanup(void);
static void setup(void);

int main(int ac, char **av)
{
	int lc;

	struct stat statbuf;
	int fildes;
	unsigned short filmode;

	/*
	 * parse standard command line options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * check looping state if -i option given on the command line
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;	/* reset tst_count while looping. */

		/* test #1 */
		TEST(open(pfilname, O_RDWR | O_CREAT, 01444));

		fildes = TEST_RETURN;
		if (fildes == -1) {
			tst_resm(TFAIL, "Cannot open %s", pfilname);
			continue;
		}

		fstat(fildes, &statbuf);
		filmode = statbuf.st_mode;
		if (!(filmode & S_ISVTX)) {
			tst_resm(TFAIL, "Save test bit cleared, but "
				 "should not have been");
		} else {
			tst_resm(TPASS, "Save text bit not cleared "
				 "as expected");
		}

		/* test #2 */
		TEST(open("/tmp", O_DIRECTORY));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "open of /tmp failed, errno: %d",
				 TEST_ERRNO);
			continue;
		}

		fstat(TEST_RETURN, &statbuf);
		filmode = statbuf.st_mode;
		if (!(filmode & S_IFDIR)) {
			tst_resm(TFAIL, "directory bit cleared, but "
				 "should not have been");
		} else {
			tst_resm(TPASS, "directory bit is set "
				 "as expected");
		}

		/* clean up things is case we are looping */
		if (close(fildes) == -1)
			tst_brkm(TBROK, cleanup, "close #1 failed");

		if (unlink(pfilname) == -1)
			tst_brkm(TBROK, cleanup, "can't remove file");

		if (close(TEST_RETURN) == -1)
			tst_brkm(TBROK, cleanup, "close #2 failed");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	umask(0);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(pfilname, "open3.%d", getpid());
}

static void cleanup(void)
{
	tst_rmdir();
}

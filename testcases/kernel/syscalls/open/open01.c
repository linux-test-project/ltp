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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "usctest.h"

char *TCID = "open01";
int TST_TOTAL = 1;
extern int Tst_count;

char pfilname[40] = "";

void cleanup(void);
void setup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	struct stat statbuf;
	int fildes;
	unsigned short filmode;

	/*
	 * parse standard command line options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup for test */

	/*
	 * check looping state if -i option given on the command line
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;	/* reset Tst_count while looping. */

		/* test #1 */
		TEST(open(pfilname, O_RDWR | O_CREAT, 01444));

		if ((fildes = TEST_RETURN) == -1) {
			tst_resm(TFAIL, "Cannot open %s", pfilname);
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			fstat(fildes, &statbuf);
			filmode = statbuf.st_mode;
			if (!(filmode & S_ISVTX)) {
				tst_resm(TFAIL, "Save test bit cleared, but "
					 "should not have been");
			} else {
				tst_resm(TPASS, "Save text bit not cleared "
					 "as expected");
			}
		} else {
			tst_resm(TPASS, "open call succeeded");
		}

		/* test #2 */
		TEST(open("/tmp", O_DIRECTORY));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "open of /tmp failed, errno: %d",
				 TEST_ERRNO);
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			fstat(TEST_RETURN, &statbuf);
			filmode = statbuf.st_mode;
			if (!(filmode & S_IFDIR)) {
				tst_resm(TFAIL, "directory bit cleared, but "
					 "should not have been");
			} else {
				tst_resm(TPASS, "directory bit is set "
					 "as expected");
			}
		} else {
			tst_resm(TPASS, "open of /tmp succeeded");
		}

		/* clean up things is case we are looping */
		if (close(fildes) == -1) {
			tst_brkm(TBROK, cleanup, "close #1 failed");
		}

		if (unlink(pfilname) == -1) {
			tst_brkm(TBROK, cleanup, "can't remove file");
		}

		if (close(TEST_RETURN) == -1) {
			tst_brkm(TBROK, cleanup, "close #2 failed");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	umask(0);

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that options was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	sprintf(pfilname, "open3.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 *	       premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

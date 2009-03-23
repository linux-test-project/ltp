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
 *	dup204.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of dup2(2).
 *
 * ALGORITHM
 *	attempt to call dup2() on read/write ends of a pipe
 *
 * USAGE:  <for command-line>
 *  dup204 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICTION
 *	NONE
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <test.h>
#include <usctest.h>

void setup();
void cleanup();

char *TCID = "dup204";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int Fd[2];
int NFd[2];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i, fd;
	struct stat oldbuf, newbuf;

	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(dup2(Fd[i], NFd[i]));

			if ((fd = TEST_RETURN) == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				if (fstat(Fd[i], &oldbuf) == -1) {
					tst_brkm(TBROK, cleanup, "fstat() #1 "
						 "failed");
				}
				if (fstat(NFd[i], &newbuf) == -1) {
					tst_brkm(TBROK, cleanup, "fstat() #2 "
						 "failed");
				}

				if (oldbuf.st_ino != newbuf.st_ino) {
					tst_resm(TFAIL, "original and duped "
						 "inodes do not match");
				} else {
					tst_resm(TPASS, "original and duped "
						 "inodes are the same");
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}

			/* close the duped file */
			if (close(fd) == -1) {
				tst_brkm(TBROK, cleanup, "close failed");
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* Initialize Fd in case we get a quick signal */
	Fd[0] = -1;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	if (pipe(Fd) == -1) {
		tst_brkm(TBROK, cleanup, "pipe(&Fd) Failed, errno=%d : %s",
			 errno, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	int i;

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* close the open file we've been dup'ing */
	for (i = 0; i < 2; i++) {
		if (close(Fd[i]) == -1) {
			tst_resm(TWARN, "close(%d) Failed, errno = %d "
				 ": %s", Fd[i], errno, strerror(errno));
		}
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

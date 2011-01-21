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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "dup204";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */

int fd[2];
int nfd[2];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;
	struct stat oldbuf, newbuf;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(dup2(fd[i], nfd[i]));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				if (fstat(fd[i], &oldbuf) == -1)
					tst_brkm(TBROK, cleanup, "fstat() #1 "
						 "failed");
				if (fstat(nfd[i], &newbuf) == -1)
					tst_brkm(TBROK, cleanup, "fstat() #2 "
						 "failed");

				if (oldbuf.st_ino != newbuf.st_ino)
					tst_resm(TFAIL, "original and duped "
						 "inodes do not match");
				else
					tst_resm(TPASS, "original and duped "
						 "inodes are the same");
			} else
				tst_resm(TPASS, "call succeeded");

			if (close(TEST_RETURN) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "close failed");
		}
	}
	cleanup();

	tst_exit();
}

void setup()
{
	fd[0] = -1;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if (pipe(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "pipe failed");
}

void cleanup()
{
	int i;

	TEST_CLEANUP;

	for (i = 0; i < (sizeof(fd) / sizeof(fd[0])); i++) {
		close(fd[i]);
		close(nfd[i]);
	}

	tst_rmdir();
}

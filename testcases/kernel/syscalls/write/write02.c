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
 *	write02.c
 *
 * DESCRIPTION
 *	Basic functionality test: does the return from write match the count
 *	of the number of bytes written.
 *
 *
 * ALGORITHM
 *	Create a file and write some bytes out to it.
 *	Check the return count against the number returned.
 *
 * USAGE:  <for command-line>
 *      write02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include "test.h"

char *TCID = "write02";
int TST_TOTAL = 1;

void cleanup(void);
void setup(void);

char pfiln[40] = "";

int main(int argc, char **argv)
{
	int lc;

	int cwrite;
	int fild;
	int iws;
	int badcount = 0;
	char pwbuf[BUFSIZ + 1];

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();		/* global setup for test */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

//block1:
		tst_resm(TINFO, "Block 1: test to see write() returns proper "
			 "write count");

		for (iws = 0; iws < BUFSIZ; iws++) {
			pwbuf[iws] = 'A' + (iws % 26);
		}
		pwbuf[BUFSIZ] = '\n';

		if ((fild = creat(pfiln, 0777)) == -1) {
			tst_brkm(TBROK, cleanup, "Can't creat Xwrit");
		}
		for (iws = BUFSIZ; iws > 0; iws--) {
			if ((cwrite = write(fild, pwbuf, iws)) != iws) {
				badcount++;
				tst_resm(TINFO, "bad write count");
			}
		}
		if (badcount != 0) {
			tst_resm(TFAIL, "write() FAILED to return proper cnt");
		} else {
			tst_resm(TPASS, "write() PASSED");
		}
		tst_resm(TINFO, "block 1 passed");
		close(fild);
	}
	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	tst_tmpdir();

	sprintf(pfiln, "write1.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 * premature exit.
 */
void cleanup(void)
{

	unlink(pfiln);

	tst_rmdir();
}

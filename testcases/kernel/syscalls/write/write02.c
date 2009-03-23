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
#include "usctest.h"

char *TCID = "write02";
int TST_TOTAL = 1;
extern int Tst_count;

void cleanup(void);
void setup(void);

char pfiln[40] = "";

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int cwrite;
	int fild;
	int iws;
	int badcount = 0;
	char pwbuf[BUFSIZ + 1];

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL))) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup for test */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

//block1:
		tst_resm(TINFO, "Block 1: test to see write() returns proper "
			 "write count");

		for (iws = 0; iws < BUFSIZ; iws++) {
			pwbuf[iws] = 'A' + (iws % 26);
		}
		pwbuf[BUFSIZ] = '\n';

		if ((fild = creat(pfiln, 0777)) == -1) {
			tst_brkm(TBROK, cleanup, "Can't creat Xwrit");
		 /*NOTREACHED*/}
		for (iws = BUFSIZ; iws > 0; iws--) {
			if ((cwrite = write(fild, pwbuf, iws)) != iws) {
				TEST_ERROR_LOG(errno);
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
	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

// Changed by prashant yendigeri, because the temp file was not being created in//  the $TDIRECTORY
//      sprintf(pfiln, "./write1.%d", getpid());
	sprintf(pfiln, "write1.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 * premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(pfiln);

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

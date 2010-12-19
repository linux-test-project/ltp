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
 *	open04.c
 *
 * DESCRIPTION
 *	Testcase to check that open(2) sets EMFILE if a process opens files
 *	more than its descriptor size
 *
 * ALGORITHM
 *	First get the file descriptor table size which is set for a process.
 *	Use open(2) for creating files till the descriptor table becomes full.
 *	These open(2)s should succeed. Finally use open(2) to open another
 *	file. This attempt should fail with EMFILE.
 *
 * USAGE:  <for command-line>
 *  open04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 *
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "open04";
int TST_TOTAL = 1;

int fd, ifile, mypid, first;
int nfile;
int *buf;
char fname[40];

int exp_enos[] = { EMFILE, 0 };

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(open(fname, O_RDWR | O_CREAT, 0777));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != EMFILE) {
			tst_resm(TFAIL, "Expected EMFILE, got %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "call returned expected EMFILE error");
		}
	}
	close(first);
	close(fd);
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	mypid = getpid();
	nfile = getdtablesize();
	sprintf(fname, "open04.%d", mypid);

	if ((first = fd = open(fname, O_RDWR | O_CREAT, 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "Cannot open first file");
	}

	close(fd);
	close(first);
	unlink(fname);

	/* Allocate memory for stat and ustat structure variables */
	if ((buf = (int *)malloc(sizeof(int) * nfile - first)) == NULL) {
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");
	}

	for (ifile = first; ifile <= nfile; ifile++) {
		sprintf(fname, "open04.%d.%d", ifile, mypid);
		if ((fd = open(fname, O_RDWR | O_CREAT, 0777)) == -1) {
			if (errno != EMFILE) {
				tst_brkm(TBROK, cleanup, "Expected EMFILE got "
					 "%d", errno);
			}
			break;
		}
		buf[ifile - first] = fd;
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(first);

	TEST_CLEANUP;

	for (ifile = first; ifile < nfile; ifile++) {
		sprintf(fname, "open04.%d.%d", ifile, mypid);
		close(buf[ifile - first]);
		unlink(fname);
	}

	/* delete the test directory created in setup() */
	tst_rmdir();

}
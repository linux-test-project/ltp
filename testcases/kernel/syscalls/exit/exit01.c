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
 *	exit01.c
 *
 * DESCRIPTION
 *	Check that exit returns the correct values to the waiting parent
 *
 * ALGORITHM
 * 	Fork a process that immediately calls exit() with a known
 * 	value. Check for that value in the parent.
 *
 * USAGE
 * 	exit01
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID = "exit01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int pid, npid, sig, nsig, exno, nexno, status;
	int rval = 0;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSIkNG ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup for test */

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		sig = 0;
		exno = 1;

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "fork() failed");

		if (pid == 0) {	/* parent */
			exit(exno);
		} else {
			sleep(1);	/* let child start */
			npid = wait(&status);

			if (npid != pid) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected pid returned");
				rval = 1;
			}

			nsig = status % 256;

			/*
			 * Check if the core dump bit has been set, bit # 7
			 */
			if (nsig >= 128) {
				nsig = nsig - 128;
			}

			/*
			 * nsig is the signal number returned by wait
			 */
			if (nsig != sig) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected signal returned");
				rval = 1;
			}

			/*
			 * nexno is the exit number returned by wait
			 */
			nexno = status / 256;
			if (nexno != exno) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected exit number returned");
				rval = 1;
			}
		}

		if (rval != 1) {
			tst_resm(TPASS, "exit() test PASSED");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion or
 * 	       premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

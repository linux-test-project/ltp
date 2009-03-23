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
 *	setpgrp02.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the setpgrp(2) syscall.
 *
 * ALGORITHM
 *	Check the values that setpgrp() and getpgrp() return. The setpgrp()
 *	returns 0 on success in Linux, but, in DYNIX/ptx this call returns
 *	the new pgid.
 *
 * USAGE:  <for command-line>
 *  setpgrp02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <wait.h>
#include "test.h"
#include "usctest.h"

char *TCID = "setpgrp02";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned by parse_opts */

	int pid, oldpgrp;
	int e_code, status, retval = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
			oldpgrp = getpgrp();

			TEST(setpgrp());

			if (TEST_RETURN != 0) {
				retval = 1;
				tst_resm(TFAIL, "setpgrp() FAILED, errno:%d",
					 errno);
				continue;
			}

			if (!STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS, "call succeeded");
				continue;
			}

			if (getpgrp() == oldpgrp) {
				retval = 1;
				tst_resm(TFAIL, "setpgrp() FAILED to set "
					 "new group id");
				continue;
			} else {
				tst_resm(TPASS, "functionality is correct");
			}
			exit(retval);
		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if ((e_code != 0) || (retval != 0)) {
				tst_resm(TFAIL, "Failures reported above");
			}
			cleanup();
		}
	}

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
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
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

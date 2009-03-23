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
 *	pipe06.c
 *
 * DESCRIPTION
 *	Check what happens when the system runs out of pipes.
 *
 * ALGORITHM
 *	Issue enough pipe calls to run the system out of pipes.
 *	Check that we get EMFILE.
 *
 * USAGE:  <for command-line>
 *  pipe06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	None
 */
#include <fcntl.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe06";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { EMFILE, 0 };

int pipe_ret, pipes[2];
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(pipe(pipes));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != EMFILE) {
			tst_resm(TFAIL, "unexpected error - %d : %s - "
				 "expected EMFILE", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "expected failure - "
				 "errno = %d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

	}
	cleanup();
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	int i, numb_fds;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	numb_fds = getdtablesize();

	for (i = 0; i < numb_fds; i++) {
		pipe_ret = pipe(pipes);
		if (pipe_ret < 0) {
			if (errno != EMFILE) {
				tst_brkm(TBROK, cleanup, "got unexpected "
					 "error - %d", errno);
			}
			break;
		}
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
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

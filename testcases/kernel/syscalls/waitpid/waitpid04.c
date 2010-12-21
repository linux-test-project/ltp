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
 *	waitpid04.c
 *
 * DESCRIPTION
 *	test to check the error conditions in waitpid sys call
 *
 * USAGE:  <for command-line>
 *      waitpid04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	NONE
 */

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

/* 0 terminated list of expected errnos */
int exp_enos[] = { 10, 22, 0 };

char *TCID = "waitpid04";
int TST_TOTAL = 1;

#define INVAL_FLAG	-1

int flag, condition_number;

int main(int ac, char **av)
{
	int pid, status, ret;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	 }

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = waitpid(pid, &status, WNOHANG);
		flag = 0;
		condition_number = 1;
		if (ret != -1) {
			tst_resm(TFAIL, "condition %d test failed",
				 condition_number);
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != ECHILD) {
				tst_resm(TFAIL, "waitpid() set invalid "
					 "errno, expected ECHILD, got: %d",
					 errno);
			} else {
				tst_resm(TPASS, "condition %d test passed",
					 condition_number);
			}
		}
		condition_number++;

		if (FORK_OR_VFORK() == 0) {
			exit(0);
		}
		pid = 1;
		ret = waitpid(pid, &status, WUNTRACED);
		flag = 0;
		if (ret != -1) {
			tst_resm(TFAIL, "condition %d test failed",
				 condition_number);
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != ECHILD) {
				tst_resm(TFAIL, "waitpid() set invalid "
					 "errno, expected ECHILD, got: %d",
					 errno);
			} else {
				tst_resm(TPASS, "condition %d test passed",
					 condition_number);
			}
		}
		condition_number++;

		/* Option is Inval = INVAL_FLAG */
		ret = waitpid(pid, &status, INVAL_FLAG);
		flag = 0;
		if (ret != -1) {
			tst_resm(TFAIL, "condition %d test failed",
				 condition_number);
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != EINVAL) {
				tst_resm(TFAIL, "waitpid() set invalid "
					 "errno, expected EINVAL, got: %d",
					 errno);
			} else {
				tst_resm(TPASS, "condition %d test passed",
					 condition_number);
			}
		}
		condition_number++;
	}
	cleanup();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	    /* Pause if that option was specified
	     * TEST_PAUSE contains the code to fork the test with the -c option.
	     */
	    TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

 }
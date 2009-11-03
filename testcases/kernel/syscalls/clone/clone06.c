/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: clone06
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test to verify inheritance of environment variables.
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Test to verify inheritance of environment variables by child.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Call clone()
 *
 *	  CHILD:
 *		get the value for environment variable, TERM  and write it
 *		to pipe.
 *
 *	  PARENT:
 *		Reads the value for environment variable, TERM from pipe.
 *		Compare child's TERM value with that of parent,
 *		if same,
 *			Test Passed.
 *		else
 *			Test failed.
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  clone06 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"
#include "clone_platform.h"

#define MAX_LINE_LENGTH 256

static void setup();
static void cleanup();
static int child_environ();

static int pfd[2];

char *TCID = "clone06";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *child_stack;	/* stack for child */
	char *parent_env;
	char buff[MAX_LINE_LENGTH];

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* Allocate stack for child */
	if ((child_stack = (void *)malloc(CHILD_STACK_SIZE)) == NULL) {
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");
	}

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Open a pipe */
		if ((pipe(pfd)) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup, "pipe() failed");
		}

		/*
		 * Call clone(2)
		 */
		TEST(ltp_clone(0, child_environ, NULL, CHILD_STACK_SIZE,
				child_stack));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "clone() failed");
			cleanup();
		}

		/* close write end from parent */
		if ((close(pfd[1])) == -1) {
			tst_resm(TWARN|TERRNO, "close(pfd[1]) failed");
		}

		/* Read env var from read end */
		if ((read(pfd[0], buff, sizeof(buff))) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup, "read from pipe failed");
		}

		/* Close read end from parent */
		if ((close(pfd[0])) == -1) {
			tst_resm(TWARN|TERRNO, "close(pfd[0]) failed");
		}

		parent_env = getenv("TERM");

		if ((strcmp(buff, parent_env)) == 0) {
			tst_resm(TPASS, "Test Passed");
		} else {
			tst_resm(TFAIL, "Test Failed");
		}
	}			/* End for TEST_LOOPING */

	free(child_stack);

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
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
}				/* End cleanup() */

/*
 * child_environ() -	function executed by child.
 *			Gets the value for environment variable,TERM &
 *			writes it to  a pipe.
 */
int child_environ(void)
{

	char var[MAX_LINE_LENGTH];

	/* Close read end from child */
	if ((close(pfd[0])) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "close(pfd[0]) failed");
	}

	if ((sprintf(var, "%s", getenv("TERM") ? : "")) <= 0) {
		tst_resm(TWARN|TERRNO, "sprintf() failed");
	}

	if ((write(pfd[1], var, MAX_LINE_LENGTH)) == -1) {
		tst_resm(TWARN|TERRNO, "write to pipe failed");
	}

	/* Close write end of pipe from child */
	if ((close(pfd[1])) == -1) {
		tst_resm(TWARN|TERRNO, "close(pfd[1]) failed");
	}

	exit(0);
}

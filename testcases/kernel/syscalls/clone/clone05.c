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
 *    TEST IDENTIFIER	: clone05
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: test for CLONE_VFORK flag
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
 *	Call clone() with CLONE_VFORK flag set. verify that
 *	execution of parent is suspended until child finishes
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call with CLONE_VM & CLONE_VFORK flags
 *
 *	CHILD:
 *		sleeps for a second, changes parent_variable to 1
 *	PARENT:
 *		If return code is not -1 and parent_variable == 1
 *			test passed
 *		else
 *			test failed
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  clone05 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 ****************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"
#include "clone_platform.h"

#define FLAG CLONE_VM | CLONE_VFORK

static void setup();
static void cleanup();
static int child_fn();

static int parent_variable = 0;

char *TCID = "clone05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *child_stack;	/* stack for child */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* Allocate stack for child */
	if ((child_stack = (void *)malloc(CHILD_STACK_SIZE)) == NULL) {
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call clone(2)
		 */
		TEST(ltp_clone(FLAG, child_fn, NULL, CHILD_STACK_SIZE,
				child_stack));

		/* check return code & parent_variable */
		if ((TEST_RETURN != -1) && (parent_variable)) {
			tst_resm(TPASS, "Test Passed");
		} else {
			tst_resm(TFAIL, "Test Failed");
		}

		/* Reset parent_variable */
		parent_variable = 0;
	}

	free(child_stack);

	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 *cleanup() -   performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}

/*
 * do_child() - function executed by child
 */
int child_fn()
{
	/*
	 * Sleep for a second, to ensure that child does not exit
	 * immediately
	 */
	sleep(1);
	parent_variable = 1;
	exit(1);
}

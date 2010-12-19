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
 *    TEST IDENTIFIER	: clone01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for clone(2)
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
 *	This is a Phase I test for the clone(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Call clone() with only SIGCHLD flag
 *
 *	  CHILD:
 *		return 0;
 *
 *	  PARENT:
 *		wait for child to finish
 *		If return values from clone() & wait() are equal & != -1
 *			test passed
 *		else
 *			test failed
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  clone01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include "test.h"
#include "usctest.h"
#include "clone_platform.h"

static void setup();
static void cleanup();
static int do_child();

char *TCID = "clone01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int ac, char **av)
{
	char *msg;		/* message returned from parse_opts */
	void *child_stack;	/* stack for child */
	int status, child_pid;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	if ((child_stack = malloc(CHILD_STACK_SIZE)) == NULL)
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");

	Tst_count = 0;

	TEST(ltp_clone(SIGCHLD, do_child, NULL, CHILD_STACK_SIZE, child_stack));

again:
	child_pid = wait(&status);

	/* check return code */
	if (TEST_RETURN == child_pid)
		tst_resm(TPASS, "clone returned %ld", TEST_RETURN);
	else if (TEST_RETURN == -1)
		tst_resm(TFAIL|TTERRNO, "clone failed for pid = %d", child_pid);
	else
		goto again;

	free(child_stack);

	cleanup();

	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

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

}

/*
 * do_child() - function executed by child
 */
int do_child(void)
{
	exit(0);
}
/*
 * Copyright (c) International Business Machines  Corp., 2003.
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
 *    TEST IDENTIFIER	: clone07
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: glibc bug test for clone(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Robbie Williamson <robbiew@us.ibm.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a test for a glibc bug for the clone(2) system call.
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
 *		If a SIGSEGV is not received by the child for using return()
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

#define TRUE 1
#define FALSE 0

static void setup();
static void cleanup();
static int do_child();

char *TCID = "clone07";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

void sigsegv_handler(int);
void sigusr2_handler(int);
static int child_pid;
static int fail = FALSE;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *child_stack;	/* stack for child */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;
                               /* Allocate stack for child */
                if((child_stack = (void *) malloc(CHILD_STACK_SIZE)) == NULL) {
                        tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");
                }


		/*
		 * Call clone(2)
		 */
		child_pid = ltp_clone(SIGCHLD, do_child, NULL,
				CHILD_STACK_SIZE, child_stack);
		wait(NULL);
		free(child_stack);
	}			/* End for TEST_LOOPING */

	if (fail == FALSE)
		tst_resm(TPASS,
			 "Use of return() in child did not cause SIGSEGV");
	else {
		tst_resm(TFAIL, "Use of return() in child caused SIGSEGV");
	}

	cleanup();

	return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	struct sigaction def_act;
	struct sigaction act;

	/* Pause if that option was specified */
	TEST_PAUSE;

	act.sa_handler = sigsegv_handler;
	act.sa_flags = SA_RESTART;
	if ((sigaction(SIGSEGV, &act, NULL)) == -1) {
		tst_resm(TWARN|TERRNO,
			 "sigaction() for SIGSEGV failed in test_setup()");
	}

	/* Setup signal handler for SIGUSR2 */
	def_act.sa_handler = sigusr2_handler;
	def_act.sa_flags = SA_RESTART | SA_RESETHAND;

	if ((sigaction(SIGUSR2, &def_act, NULL)) == -1) {
		tst_resm(TWARN|TERRNO,
			 "sigaction() for SIGUSR2 failed in test_setup()");
	}

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

	kill(child_pid, SIGKILL);
	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

/*
 * do_child() - function executed by child
 */
int do_child(void)
{
	return 0;
}

void sigsegv_handler(int sig)
{
	if (child_pid == 0) {
		kill(getppid(), SIGUSR2);
		_exit(42);
	}
}

/* sig_default_handler() - Default handler for parent */
void sigusr2_handler(int sig)
{
	if (child_pid != 0) {
		fail = TRUE;
	}
}

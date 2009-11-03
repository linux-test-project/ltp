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
 *    TEST IDENTIFIER	: ptrace02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: functionality test for ptrace(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case tests the functionality of ptrace() for
 *	PTRACE_TRACEME & PTRACE_CONT requests.
 *	Here, we fork a child & the child does ptrace(PTRACE_TRACEME, ...).
 *	Then a signal is delivered to the child & verified that parent
 *	is notified via wait(). then parent does ptrace(PTRACE_CONT, ..)
 *	to make the child to continue. Again parent wait() for child to finish.
 *	If child finished normally, test passes.
 *		We test two cases
 *			1) By telling child to ignore SIGUSR2 signal
 *			2) By installing a signal handler for child for SIGUSR2
 *		In both cases, child should stop & notify parent on reception
 *		of SIGUSR2
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup signal handler for SIGUSR2 signal
 *	 fork a child
 *
 *	 CHILD:
 *		setup signal handler for SIGUSR2 signal
 *		call ptrace() with PTRACE_TRACEME request
 *		send SIGUSR2 signal to self
 *	 PARENT:
 *		wait() for child.
 *		if parent is notified when child gets a signal through wait(),
 *		then
 *			do  ptrace(PTRACE_CONT, ..) on child
 * 			wait() for child to finish,
 * 			if child exited normaly,
 *				TEST passed
 * 			else
 * 				TEST failed
 *		else
 *			TEST failed
 *	
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  ptrace02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <config.h>
#include "ptrace.h"

#include "test.h"
#include "usctest.h"

static void do_child(void);
static void setup(void);
static void cleanup(void);
static void child_handler();
static void parent_handler();

static int got_signal = 0;

char *TCID = "ptrace02";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static int i;			/* loop test case counter, shared with do_child */

int TST_TOTAL = 2;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &i);
#endif

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			got_signal = 0;

			/* Setup signal handler for parent */
			if (i == 1) {
				parent_act.sa_handler = parent_handler;
				parent_act.sa_flags = SA_RESTART;

				if ((sigaction(SIGUSR2, &parent_act, NULL))
				    == -1) {
					tst_resm(TWARN, "sigaction() failed"
						 " in parent");
					continue;
				}
			}

			switch (child_pid = FORK_OR_VFORK()) {

			case -1:
				/* fork() failed */
				tst_resm(TFAIL, "fork() failed");
				continue;

			case 0:
				/* Child */
#ifdef UCLINUX
				if (self_exec(av[0], "d", i) < 0) {
					tst_resm(TFAIL, "self_exec failed");
					continue;
				}
#else
				do_child();
#endif

			default:
				/* Parent */
				if ((waitpid(child_pid, &status, 0)) < 0) {
					tst_resm(TFAIL, "waitpid() failed");
					continue;
				}

				/*
				 * Check the exit status of child. If (it exits
				 * normally with exit value 1) OR (child came
				 * through signal handler), Test Failed
				 */

				if (((WIFEXITED(status)) &&
				     (WEXITSTATUS(status))) ||
				    (got_signal == 1)) {
					tst_resm(TFAIL, "Test Failed");
					continue;
				} else {
					/* Restart child */
					if ((ptrace(PTRACE_CONT, child_pid,
						    0, 0)) == -1) {
						tst_resm(TFAIL, "Test Failed:"
							 " Parent was not able to do"
							 " ptrace(PTRACE_CONT, ..) on"
							 " child");
						continue;
					}
				}

				if ((waitpid(child_pid, &status, 0)) < 0) {
					tst_resm(TFAIL, "waitpid() failed");
					continue;
				}

				if (WIFEXITED(status)) {
					/* Child exits normally */
					tst_resm(TPASS, "Test Passed");
				} else {
					tst_resm(TFAIL, "Test Failed");
				}

			}
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* do_child() */
void do_child()
{
	struct sigaction child_act;

	/* Setup signal handler for child */
	if (i == 0) {
		child_act.sa_handler = SIG_IGN;
	} else {
		child_act.sa_handler = child_handler;
	}
	child_act.sa_flags = SA_RESTART;

	if ((sigaction(SIGUSR2, &child_act, NULL)) == -1) {
		tst_resm(TWARN, "sigaction() failed in child");
		exit(1);
	}

	if ((ptrace(PTRACE_TRACEME, 0, 0, 0)) == -1) {
		tst_resm(TWARN, "ptrace() failed in child");
		exit(1);
	}

	/* ensure that child bypasses signal handler */
	if ((kill(getpid(), SIGUSR2)) == -1) {
		tst_resm(TWARN, "kill() failed in child");
		exit(1);
	}
	exit(1);
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

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
 * child_handler() - Signal handler for child
 */
void child_handler()
{

	if ((kill(getppid(), SIGUSR2)) == -1) {
		tst_resm(TWARN, "kill() failed in child_handler()");
		exit(1);
	}
}

/*
 * parent_handler() - Signal handler for parent
 */
void parent_handler()
{

	got_signal = 1;
}

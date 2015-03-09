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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

static void do_child(void);
static void setup(void);
static void cleanup(void);
static void child_handler();
static void parent_handler();

static int got_signal = 0;

char *TCID = "ptrace02";
static int i;			/* loop test case counter, shared with do_child */

int TST_TOTAL = 2;

int main(int ac, char **av)
{

	int lc;
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "d", &i);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			got_signal = 0;

			/* Setup signal handler for parent */
			if (i == 1) {
				parent_act.sa_handler = parent_handler;
				parent_act.sa_flags = SA_RESTART;
				sigemptyset(&parent_act.sa_mask);

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
	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/* do_child() */
void do_child(void)
{
	struct sigaction child_act;

	/* Setup signal handler for child */
	if (i == 0) {
		child_act.sa_handler = SIG_IGN;
	} else {
		child_act.sa_handler = child_handler;
	}
	child_act.sa_flags = SA_RESTART;
	sigemptyset(&child_act.sa_mask);

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
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{

}

/*
 * child_handler() - Signal handler for child
 */
void child_handler(void)
{

	if ((kill(getppid(), SIGUSR2)) == -1) {
		tst_resm(TWARN, "kill() failed in child_handler()");
		exit(1);
	}
}

/*
 * parent_handler() - Signal handler for parent
 */
void parent_handler(void)
{

	got_signal = 1;
}

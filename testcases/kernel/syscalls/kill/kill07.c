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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	kill07.c
 *
 * DESCRIPTION
 *	Test case to check that SIGKILL can not be caught.
 *
 * ALGORITHM
 *	call setup
 *		setup some shared memory
 *	loop if the -i option was given
 *	set up to catch SIGKILL
 *	if SIGKILL is caught set the shared memory flag.
 *	fork a child
 *	execute the kill system call
 *	check the return value
 *	if return value is -1
 *		issue a FAIL message, break remaining tests and cleanup
 *	if we are doing functional testing
 *		if the process was terminated with the expected signal and the
 *		signal was not caught.
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE
 *  kill07 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	This test should be run as a non-root user.
 */

#include "test.h"

#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

void cleanup(void);
void setup(void);
void sighandler(int sig);
void do_child(void);

char *TCID = "kill07";
int TST_TOTAL = 1;
int shmid1;
extern key_t semkey;
int *flag;

extern int getipckey();
extern void rm_shm(int);

#define TEST_SIG SIGKILL

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
	int exno, status, nsig, asig, ret;
	struct sigaction my_act, old_act;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;
		status = 1;
		exno = 1;
		my_act.sa_handler = sighandler;
		my_act.sa_flags = SA_RESTART;
		sigemptyset(&my_act.sa_mask);

		if ((shmid1 = shmget(semkey, (int)getpagesize(),
				     0666 | IPC_CREAT)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "Failed to setup shared memory");
		}

		if (*(flag = shmat(shmid1, 0, 0)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "Failed to attatch shared memory:%d", *flag);
		}

		*flag = 0;

		/* setup the signal handler */
		ret = sigaction(TEST_SIG, &my_act, &old_act);

		pid = FORK_OR_VFORK();
		if (pid < 0) {
			tst_brkm(TBROK, cleanup, "Fork of child failed");
		} else if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup,
					 "self_exec of child failed");
			}
#else
			do_child();
#endif
		} else {
			/* sighandler should not catch this signal */
			/* if it does flag will be set to 1 */
			sleep(1);
			TEST(kill(pid, TEST_SIG));
			waitpid(pid, &status, 0);
		}

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL, cleanup, "%s failed - errno = %d : %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		}

		/*
		 * Check to see if the process was terminated with the
		 * expected signal.
		 */
		nsig = WTERMSIG(status);
		asig = WIFSIGNALED(status);
		if ((asig == 0) & (*flag == 1)) {
			tst_resm(TFAIL, "SIGKILL was unexpectedly"
				 " caught");
		} else if ((asig == 1) & (nsig == TEST_SIG)) {
			tst_resm(TINFO, "received expected signal %d",
				 nsig);
			tst_resm(TPASS,
				 "Did not catch signal as expected");
		} else if (nsig) {
			tst_resm(TFAIL,
				 "expected signal %d received %d",
				 TEST_SIG, nsig);
		} else {
			tst_resm(TFAIL, "No signals received");
		}

		if (shmdt(flag)) {
			tst_brkm(TBROK, cleanup, "shmdt failed ");
		}
	}

	cleanup();
	tst_exit();
}

/*
 * sighandler() - try to catch SIGKILL
 */

void sighandler(int sig)
{
	/* do nothing */
	*flag = 1;
	return;
}

/*
 * do_child()
 */
void do_child(void)
{
	int exno = 1;

	sleep(300);
	tst_resm(TINFO, "Child never received a signal");
	exit(exno);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	semkey = getipckey();

}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{

	/*
	 * remove the shared memory
	 */
	rm_shm(shmid1);

	tst_rmdir();

}

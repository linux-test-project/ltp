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
 * 	waitpid10.c
 *
 * DESCRIPTION
 * 	Tests to see if pid's returned from fork and waitpid are same
 *
 * ALGORITHM
 * 	Set up to catch SIGINTs, SIGALRMs, and the real time timer.
 * 	Until the timer interrupts, do the following.  Fork 8 kids.
 * 	2 will immediately exit, 2 will sleep, 2 will be compute bound,
 * 	and 2 will fork another child, both which will do mkdirs on
 * 	the same directory 50 times.  When the timer expires, kill all
 * 	kids and remove the directory.
 *
 * USAGE:  <for command-line>
 *      waitpid10 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * NOTE
 * 	This test was designed to see if the intermittant occurrance
 * 	of a waitpid returning a pid different from that returned by the
 * 	fork can be reproduced.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 * 	None
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>

#define	MAXKIDS	8

char *TCID = "waitpid10";
int TST_TOTAL = 1;
extern int Tst_count;

int alrmintr;
volatile int intintr;

void setup(void);
void cleanup(void);
void inthandlr();
void alrmhandlr();
void wait_for_parent();
void do_exit();
void do_compute();
void do_fork();
void do_sleep();
void do_mkdir();

int fail;

#ifdef UCLINUX
static char *argv0;
#endif

int main(int ac, char **av)
{
	int kid_count, ret_val, status, nkids;
	int i, j, k, found;
	int fork_kid_pid[MAXKIDS], wait_kid_pid[MAXKIDS];
	int runtime;		/* time(sec) to run this process */

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	argv0 = av[0];

	maybe_run_child(&do_exit, "n", 1);
	maybe_run_child(&do_compute, "n", 2);
	maybe_run_child(&do_fork, "n", 3);
	maybe_run_child(&do_sleep, "n", 4);
	maybe_run_child(&do_mkdir, "n", 5);
#endif

	/*
	 * process the arg -- If there is one arg, it is the
	 * number of seconds to run.  If there is no arg the program
	 * defaults to 60 sec runtime.
	 */
	if (ac == 2) {
		if (sscanf(av[1], "%d", &runtime) != 1) {
			tst_resm(TFAIL, "%s is an invalid argument", av[1]);
			tst_exit();
		}
	} else {
		runtime = 60;
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;
		fail = 0;

		if (signal(SIGALRM, alrmhandlr) == SIG_ERR) {
			tst_resm(TFAIL, "signal SIGALRM failed.  errno = %d",
				 errno);
			tst_exit();
		}
		alrmintr = 0;

		/*
		 * Set up to catch SIGINT. The kids will wait till a SIGINT
		 * has been received before they proceed.
		 */
		if (signal(SIGINT, inthandlr) == SIG_ERR) {
			tst_resm(TFAIL, "signal SIGINT failed.  errno = %d",
				 errno);
			tst_exit();
		}
		intintr = 0;

		/* Turn on the real time interval timer. */
		if ((alarm(runtime)) < 0) {
			tst_resm(TFAIL, "alarm failed.  errno = %d", errno);
			tst_exit();
		}

		/* Run the test over and over until the timer expires */
		for (;;) {
			if (alrmintr) {
				break;
			}
			/*
			 * Fork 8 kids. There will be 4 sets of 2 processes
			 * doing the same thing. Save all kid pid's in an
			 * array for future use. The kids will first wait for
			 * the parent to send SIGINT. Then will proceed to
			 * their assigned tasks.
			 */
			kid_count = 0;
			/*
			 * Clearing the intinitr flag here for all the children.
			 * So that we may not miss any signals !
			 */
			intintr = 0;
			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 0 */
				//      intintr = 0;$
#ifdef UCLINUX
				if (self_exec(argv0, "n", 1) < 0) {
					tst_resm(TFAIL, "self_exec 0 failed");
					tst_exit();
				}
#else
				do_exit();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 0 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 1 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 1) < 0) {
					tst_resm(TFAIL, "self_exec 1 failed");
					tst_exit();
				}
#else
				do_exit();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 1 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 2 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 2) < 0) {
					tst_resm(TFAIL, "self_exec 2 failed");
					tst_exit();
				}
#else
				do_compute();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 2 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 3 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 2) < 0) {
					tst_resm(TFAIL, "self_exec 3 failed");
					tst_exit();
				}
#else
				do_compute();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 3 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 4 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 3) < 0) {
					tst_resm(TFAIL, "self_exec 4 failed");
					tst_exit();
				}
#else
				do_fork();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 4 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 5 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 3) < 0) {
					tst_resm(TFAIL, "self_exec 5 failed");
					tst_exit();
				}
#else
				do_fork();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 5 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 6 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 4) < 0) {
					tst_resm(TFAIL, "self_exec 6 failed");
					tst_exit();
				}
#else
				do_sleep();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 6 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			ret_val = FORK_OR_VFORK();
			if (ret_val == 0) {	/* child 7 */
				//      intintr = 0;
#ifdef UCLINUX
				if (self_exec(argv0, "n", 4) < 0) {
					tst_resm(TFAIL, "self_exec 7 failed");
					tst_exit();
				}
#else
				do_sleep();
#endif
			}
			if (ret_val < 0) {
				tst_resm(TFAIL, "Fork kid 7 failed. errno = "
					 "%d", errno);
				tst_exit();
			}

			/* parent */
			fork_kid_pid[kid_count++] = ret_val;

			nkids = kid_count;

			/*
			 * Now send all the kids a SIGINT to tell them to
			 * proceed. We sleep for a while first to allow the
			 * children to initialize their "intintr" variables
			 * and get set up.
			 */
			sleep(15);

			for (i = 0; i < nkids; i++) {
				if (kill(fork_kid_pid[i], SIGINT) < 0) {
					tst_resm(TFAIL, "Kill of child %d "
						 "failed, errno = %d", i,
						 errno);
					tst_exit();
				}
			}

			/* Wait till all kids have terminated. */
			kid_count = 0;
			errno = 0;
			for (i = 0; i < nkids; i++) {
				while (((ret_val = waitpid(fork_kid_pid[i],
							   &status, 0)) != -1)
				       || (errno == EINTR)) {
					if (ret_val == -1) {
						continue;
					}
					wait_kid_pid[kid_count++] = ret_val;
				}
			}

			/*
			 * Check that for every entry in the fork_kid_pid
			 * array, there is a matching pid in the
			 * wait_kid_pid array.
			 */
			for (i = 0; i < MAXKIDS; i++) {
				found = 0;
				for (j = 0; j < MAXKIDS; j++) {
					if (fork_kid_pid[i] == wait_kid_pid[j]) {
						found = 1;
						break;
					}
				}
				if (!found) {
					tst_resm(TFAIL, "Did not find a "
						 "wait_kid_pid for the "
						 "fork_kid_pid of %d",
						 fork_kid_pid[i]);
					for (k = 0; k < nkids; k++) {
						tst_resm(TFAIL,
							 "fork_kid_pid[%d] = "
							 "%d", k,
							 fork_kid_pid[k]);
					}
					for (k = 0; k < kid_count; k++) {
						tst_resm(TFAIL,
							 "wait_kid_pid[%d] = "
							 "%d", k,
							 wait_kid_pid[k]);
					}
					fail = 1;
				}
			}
		}

		/* Kill kids and remove file from do_mkdir */
		rmdir("waitpid14.ttt.ttt");

		if (fail) {
			tst_resm(TFAIL, "Test FAILED");
		} else {
			tst_resm(TPASS, "Test PASSED");
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

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

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}

void alrmhandlr()
{
	alrmintr++;
}

void inthandlr()
{
	intintr++;
}

void wait_for_parent()
{
	int testvar;

	while (!intintr) {
		testvar = 0;
	}
}

void do_exit()
{
	wait_for_parent();
	exit(3);
}

void do_compute()
{
	int i;

	wait_for_parent();

	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;

	exit(4);
}

void do_fork()
{
	int fork_pid, wait_pid;
	int status, i;

	wait_for_parent();

	/*
	 * Fork a kid.  Keep track of the kid's pid, have the kid do_mkdir,
	 * and wait for it. Compare the fork_pid with the wait_pid to be
	 * sure they are the same.
	 */
	for (i = 0; i < 50; i++) {
		fork_pid = FORK_OR_VFORK();
		if (fork_pid < 0) {
			tst_resm(TFAIL, "Fork failed");
			tst_exit();
		}
		if (fork_pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "n", 5) < 0) {
				tst_resm(TFAIL, "do_fork self_exec failed");
				tst_exit();
			}
#else
			do_mkdir();
#endif
		}

		errno = 0;
		while (((wait_pid = waitpid(fork_pid, &status, 0)) != -1) ||
		       (errno == EINTR)) {
			if (wait_pid == -1) {
				continue;
			}

			if (fork_pid != wait_pid) {
				tst_resm(TFAIL, "Didnt get a pid returned "
					 "from waitpid that matches the one "
					 "returned by fork");
				tst_resm(TFAIL, "fork pid = %d, wait pid = "
					 "%d", fork_pid, wait_pid);
				fail = 1;
			}
		}
	}

	exit(4);
}

void do_sleep()
{
	wait_for_parent();
	sleep(1);
	sleep(1);

	exit(4);
}

void do_mkdir()
{
	int ret_val;

	/*
	 * Please note that this will succeed once, and then fail. That's
	 * part of the test.
	 */
	ret_val = mkdir("waitpid14.ttt.ttt", 0777);

	exit(4);
}

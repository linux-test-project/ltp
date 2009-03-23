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
 * 	setpgid03.c
 *
 * DESCRIPTION
 * 	Test to check the error and trivial conditions in setpgid system call
 *
 * USAGE
 * 	setuid03
 *
 * RESTRICTIONS
 * 	This test is not completely written in the LTP format - PLEASE FIX!
 */

#define DEBUG 0

#include <wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "setpgid03";
int TST_TOTAL = 1;
extern int Tst_count;

void do_child(void);
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int pid;
	int rval, fail = 0;
	int ret, status;
	int exno = 0;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	/*
	 * perform global setup for the test
	 */
	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

//test1:
		/* sid of the calling process is not same */
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		}

		if (pid == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "") < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		} else {	/* parent */
			sleep(1);
			rval = setpgid(pid, getppid());
			if (errno == EPERM) {
				tst_resm(TPASS, "setpgid SUCCESS to set "
					 "errno to EPERM");
			} else {
				tst_resm(TFAIL, "setpgid FAILED, "
					 "expect %d, return %d", EPERM, errno);
				fail = 1;
			}
			sleep(1);
			if ((ret = wait(&status)) > 0) {
				if (DEBUG)
					tst_resm(TINFO, "Test {%d} exited "
						 "status 0x%0x", ret, status);

				if (status != 0) {
					fail = 1;
				}
			}
		}
		if (DEBUG) {
			if (fail || exno) {
				tst_resm(TINFO, "Test test 1: FAILED");
			} else {
				tst_resm(TINFO, "Test test 1: PASSED");
			}
		}
//test2:
		/*
		 * Value of pid matches the pid of the child process and
		 * the child process has exec successfully. Error
		 */
		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_resm(TFAIL, "Fork failed");
			tst_exit();
		}
		if (pid == 0) {
			if (execlp("sleep", "sleep", "3", NULL) < 0) {
				perror("exec failed");
			}
			exit(127);
		} else {
			sleep(1);
			rval = setpgid(pid, getppid());
			if (errno == EACCES) {
				tst_resm(TPASS, "setpgid SUCCEEDED to set "
					 "errno to EACCES");
			} else {
				tst_resm(TFAIL, "setpgid FAILED, expect EACCES "
					 "got %d", errno);
			}
			if ((ret = wait(&status)) > 0) {
				if (DEBUG)
					tst_resm(TINFO, "Test {%d} exited "
						 "status 0x%0x", ret, status);
				if (status != 0) {
					fail = 1;
				}
			}
		}
	}
	cleanup();
	return 0;
}

/*
 * do_child()
 */
void do_child()
{
	int exno = 0;

	if (setsid() < 0) {
		tst_resm(TFAIL, "setsid failed, errno :%d", errno);
		exno = 1;
	}
	sleep(2);
	exit(exno);
}

/*
 * setup()
 * 	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup()
 * 	performs all the ONE TIME cleanup for this test at completion
 * 	or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

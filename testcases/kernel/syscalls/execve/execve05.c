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
 * 	execve05.c
 *
 * DESCRIPTION
 * 	This testcase tests the basic functionality of the execve(2) system
 *	call.
 *
 * ALGORITHM
 *	This program also gets the names "test1", and "test2". This tests
 *	the functionality of the execve(2) system call by spawning a few
 *	children, each of which would execute "test1/test2" executables, and
 *	finally the parent ensures that they terminated correctly.
 *
 * USAGE
 *	execve05 20 test1 test2 4
 *
 * RESTRICTIONS
 * 	This program does not follow the LTP format - *PLEASE FIX*
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

#undef DEBUG			/* change this to #define if needed */

void setup(void);
void cleanup(void);

char *TCID = "execve05";
int TST_TOTAL = 1;
extern int Tst_count;

int iterations;
char *fname1;
char *fname2;
char *prog;
char *av[6];
char *ev[1];

void usage(void)
{
	tst_resm(TBROK, "usage: %s <iters> <fname1> <fname2> <count>", TCID);
	tst_resm(TINFO, "example: %s 20 test1 test2 4", TCID);
	tst_exit();
}

int main(int ac, char **av)
{
	char iter[20];
	int pid, child, status, count;
	int nchild, i, fail = 0;

	int lc;
	char *msg;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}
	setup();

	if (ac != 5) {
		tst_resm(TINFO, "Wrong number of arguments");
		usage();
	 /*NOTREACHED*/}

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		prog = av[0];
		iterations = atoi(av[1]);
		fname1 = av[2];
		fname2 = av[3];
		count = atoi(av[4]);
#ifdef DEBUG
		tst_resm(TINFO, "Entered %s %d %s %s %d -- pid = %d", prog,
			 iterations, fname1, fname2, count, getpid());
#endif

		if (iterations == 0) {
			tst_resm(TPASS, "Test DONE, pid %d, -- %s %d %s %s",
				 getpid(), prog, iterations, fname1, fname2);
			tst_exit();
		}

		if (!count) {
			sprintf(iter, "%d", iterations - 1);
			av[0] = fname1;
			av[1] = iter;
			av[2] = fname1;
			av[3] = fname2;
			av[4] = "0";
			av[5] = 0;
			ev[0] = 0;
#ifdef DEBUG
			tst_resm(TINFO, "Doing execve(%s, av, ev)", fname1);
			tst_resm(TINFO, "av[0,1,2,3,4] = %s, %s, %s, %s, %s",
				 av[0], av[1], av[2], av[3], av[4]);
#endif
			(void)execve(fname1, av, ev);
			tst_resm(TFAIL, "Execve fail, %s, errno=%d", fname1,
				 errno);
		}

		nchild = count * 2;

		sprintf(iter, "%d", iterations);
		for (i = 0; i < count; i++) {
			pid = FORK_OR_VFORK();
			if (pid < 0) {
				perror("Fork failed");
				exit(1);
			} else if (pid == 0) {
				av[0] = fname1;
				av[1] = iter;
				av[2] = fname1;
				av[3] = fname2;
				av[4] = "0";
				av[5] = 0;
				ev[0] = 0;
				(void)execve(fname1, av, ev);
				tst_resm(TFAIL, "Execve fail, %s, errno = %d",
					 fname1, errno);
				exit(2);
			}
#ifdef DEBUG
			tst_resm(TINFO, "Main - started pid %d", pid);
#endif
			pid = FORK_OR_VFORK();
			if (pid < 0) {
				perror("Fork failed");
				exit(1);
			} else if (pid == 0) {
				av[0] = fname2;
				av[1] = iter;
				av[2] = fname2;
				av[3] = fname1;
				av[4] = "0";
				av[5] = 0;
				ev[0] = 0;
				execve(fname2, av, ev);
				tst_resm(TFAIL, "Execve fail, %s, errno = %d",
					 fname2, errno);
				exit(2);
			}
#ifdef DEBUG
			tst_resm(TINFO, "Main - started pid %d", pid);
#endif
		}

		/*
		 * Wait for children to finish
		 */
		count = 0;
		while ((child = wait(&status)) > 0) {
#ifdef DEBUG
			tst_resm(TINFO, "Test [%d] exited status = 0x%x",
				 child, status);
#endif
			++count;
			if (status) {
				fail = 1;
			}
		}

		/*
		 * Should have colledcted all children
		 */
		if (count != nchild) {
			tst_resm(TFAIL, "Wrong #children waited on, count = %d",
				 count);
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "Test FAILED");
		} else {
			tst_resm(TINFO, "Test PASSED");
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup - performs all ONE TIME steup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	umask(0);
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion or
 * 	       premature exit
 */
void cleanup(void)
{
	TEST_CLEANUP;

	tst_exit();
}

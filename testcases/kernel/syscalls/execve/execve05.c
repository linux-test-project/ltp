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
#include "safe_macros.h"

#undef DEBUG			/* change this to #define if needed */

void setup(void);
void cleanup(void);

char *TCID = "execve05";
int TST_TOTAL = 1;

int iterations;
char *fname1;
char *fname2;
char *prog;
char *av[6];
char *ev[1];

void usage(void)
{
	tst_brkm(TBROK, NULL, "usage: %s <iters> <fname1> <fname2> <count>",
		 TCID);
}

int main(int ac, char **av)
{
	char iter[20];
	int count, i, nchild, status;
	pid_t pid;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	if (ac != 5)
		usage();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

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
			tst_resm(TINFO, "doing execve(%s, av, ev)", fname1);
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
			if (pid == -1) {
				perror("fork failed");
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
				perror("execve failed");
				exit(2);
			}
#ifdef DEBUG
			tst_resm(TINFO, "Main - started pid %d", pid);
#endif
			SAFE_WAIT(cleanup, &status);
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_resm(TFAIL, "child exited abnormally");

			pid = FORK_OR_VFORK();
			if (pid == -1) {
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
				perror("execve failed");
				exit(2);
			}
#ifdef DEBUG
			tst_resm(TINFO, "Main - started pid %d", pid);
#endif
			SAFE_WAIT(cleanup, &status);
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_resm(TFAIL, "child exited abnormally");

		}

		if (wait(&status) != -1)
			tst_brkm(TBROK, cleanup,
				 "leftover children haven't exited yet");

	}
	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	umask(0);
}

void cleanup(void)
{
}

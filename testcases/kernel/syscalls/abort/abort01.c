/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	abort
 *
 * CALLS
 *	abort(3)
 *
 * ALGORITHM
 *	Fork child.  Have child abort, check return status.
 *
 * RESTRICTIONS
 *      The ulimit for core file size must be greater than 0.
 *
 * CHANGE LOG:
 * Nov 11 2002: Ported to LTP Suite by Ananda
 *
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"
#define ITER	3
#define FAILED 0
#define PASSED 1

char *TCID = "abort01";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;

void setup(void)
{
	temp = stderr;
	tst_tmpdir();
}

void cleanup(void)
{
	unlink("core");
	tst_rmdir();
}

int instress();
void setup();
int forkfail();
void do_child();

/*************/

/*--------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	register int i;
	int status, count, child, kidpid;
	int sig, ex;
	char *msg;

#ifdef WCOREDUMP
	int core;

	core = 0;
#endif
	ex = sig = 0;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (i = 0; i < ITER; i++) {

		if ((kidpid = FORK_OR_VFORK()) == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "")) {
				if (!instress()) {
					perror("fork failed");
					exit(1);
				}
			}
#else
			do_child();
#endif
		}
		if (kidpid < 0)
			if (!instress())
				tst_brkm(TBROK|TERRNO, cleanup, "fork failed");
		count = 0;
		while ((child = wait(&status)) > 0)
			count++;
		if (count != 1) {
			tst_brkm(TBROK, cleanup,
			    "wrong # children waited on; got %d, expected 1",
			    count);
		}
		if (WIFSIGNALED(status)) {

#ifdef WCOREDUMP
			core = WCOREDUMP(status);
#endif
			sig = WTERMSIG(status);

		}
		if (WIFEXITED(status))
			ex = WEXITSTATUS(status);

#ifdef WCOREDUMP
		if (core == 0) {
			tst_brkm(TFAIL, cleanup,
			    "Child did not dump core; exit code = %d, "
			    "signal = %d", ex, sig);
		} else if (core != -1)
			tst_resm(TPASS, "abort dumped core");
#endif

		if (sig == SIGIOT)
			tst_resm(TPASS, "abort raised SIGIOT");
		else {
			tst_brkm(TFAIL, cleanup,
			    "Child did not raise SIGIOT (%d); exit code = %d, "
			    "signal = %d", SIGIOT, ex, sig);
		}

	}

	cleanup();
	tst_exit();
}

void do_child()
{
	abort();
	fprintf(stderr, "\tchild - abort failed.\n");
	exit(1);
}

int instress()
{
	tst_resm(TINFO,
		 "System resources may be too low; fork(), select() etc are likely to fail.");
	return 1;
}

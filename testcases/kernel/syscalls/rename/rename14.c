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

/* 11/12/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	rename14.c - create and rename files
 *
 * CALLS
 *	create, unlink, rename
 *
 * ALGORITHM
 *	Creates two processes.  One creates and unlinks a file.
 *	The other renames that file.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/** LTP Port **/
#include "test.h"
#include "usctest.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;

char *TCID = "rename14";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
/**************/

#define RUNTIME	45

int kidpid[2];
int parent_pid;

int main(argc, argv)
int argc;
char *argv[];
{
	int pid;
	sigset_t set;
	struct sigaction act, oact;
	int term();
	int al();
	void dochild1();
	void dochild2();

#ifdef UCLINUX
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	maybe_run_child(&dochild1, "n", 1);
	maybe_run_child(&dochild2, "n", 2);
#endif
	sigemptyset(&set);
	act.sa_handler = (void (*)())term;
	act.sa_mask = set;
	act.sa_flags = 0;
	if (sigaction(SIGTERM, &act, &oact)) {
		tst_resm(TBROK, "Sigaction(SIGTERM)");
		tst_exit();
	}

	sigemptyset(&set);
	act.sa_handler = (void (*)())al;
	act.sa_mask = set;
	act.sa_flags = 0;
	if (sigaction(SIGALRM, &act, 0)) {
		tst_resm(TBROK, "Sigaction(SIGALRM)");
		tst_exit();
	}
	parent_pid = getpid();
	tst_tmpdir();
/*--------------------------------------------------------------*/

	pid = FORK_OR_VFORK();
	if (pid < 0) {
		tst_resm(TBROK, "fork() returned %d", pid);
		tst_exit();
	}
	if (pid == 0) {
#ifdef UCLINUX
		if (self_exec(argv[0], "n", 1) < 0) {
			tst_resm(TBROK, "self_exec failed");
		}
#else
		dochild1();
#endif
	}
	kidpid[0] = pid;
	pid = FORK_OR_VFORK();
	if (pid < 0) {
		(void)kill(kidpid[0], SIGTERM);
		(void)unlink("./rename14");
		tst_resm(TBROK, "fork() returned %d", pid);
		tst_exit();
	}
	if (pid == 0) {
#ifdef UCLINUX
		if (self_exec(argv[0], "n", 1) < 0) {
			tst_resm(TBROK, "self_exec failed");
		}
#else
		dochild2();
#endif
	}
	kidpid[1] = pid;

	alarm(RUNTIME);

	/* Collect child processes. */
	/* Wait for timeout */
	pause();

	kill(kidpid[0], SIGTERM);
	kill(kidpid[1], SIGTERM);

	waitpid(kidpid[0], NULL, 0);
	waitpid(kidpid[1], NULL, 0);

	unlink("./rename14");
	unlink("./rename14xyz");
	(local_flag == PASSED) ? tst_resm(TPASS, "Test Passed")
	    : tst_resm(TFAIL, "Test Failed");

	tst_rmdir();
/*--------------------------------------------------------------*/
	tst_exit();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
/*--------------------------------------------------------------*/
	return 0;
}

/* FUNCTIONS GO HERE */

int term()
{
	if (parent_pid != getpid())
		exit(0);
	if (kidpid[0])
		return (kill(kidpid[0], SIGTERM));
	if (kidpid[1])
		return (kill(kidpid[1], SIGTERM));
	return 0;
}

int al()
{
	if (kidpid[0])
		return (kill(kidpid[0], SIGTERM));
	if (kidpid[1])
		return (kill(kidpid[1], SIGTERM));
	return 0;
}

void dochild1()
{
	int fd;

	for (;;) {
		fd = creat("./rename14", 0666);
		unlink("./rename14");
		close(fd);
	}
}

void dochild2()
{
	for (;;) {
		rename("./rename14", "./rename14xyz");
	}
}

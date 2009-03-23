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
 *	pipe11.c
 *
 * DESCRIPTION
 *	Check if many children can read what is written to a pipe by the
 *	parent.
 *
 * ALGORITHM
 *	1. Open a pipe and write to it
 *	2. Fork a large number of children
 *	3. Have the children read the pipe and check how many characters
 *	   each got
 *
 * USAGE:  <for command-line>
 *  pipe11 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe11";
int TST_TOTAL = 1;
extern int Tst_count;

void do_child(void);
void do_child_uclinux(void);
void setup(void);
void cleanup(void);

#define	NUMCHILD	50
#define	NCPERCHILD	50
char rawchars[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
int kidid;
int numchild;			/* no of children to fork */
int ncperchild;			/* no of chars child should read */
int szcharbuf;			/* size of char buf */
int pipewrcnt;			/* chars written to pipe */
char *wrbuf, *rdbuf;
int fd[2];			/* fds for pipe read/write */

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int i;
	int fork_ret, status;
	int written;		/* no of chars read and written */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}
#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "ddddd", &fd[0], &fd[1], &kidid,
			&ncperchild, &szcharbuf);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(pipe(fd));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "pipe creation failed");
			continue;
		}

		if (!STD_FUNCTIONAL_TEST) {
			tst_resm(TPASS, "call succeeded");
			continue;
		}

		written = write(fd[1], wrbuf, szcharbuf);
		if (written != szcharbuf) {
			tst_brkm(TBROK, cleanup, "write to pipe failed");
		}

	      refork:
		++kidid;
		fork_ret = FORK_OR_VFORK();

		if (fork_ret < 0) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		 /*NOTREACHED*/}

		if ((fork_ret != 0) && (fork_ret != -1) && (kidid < numchild)) {
			goto refork;
		}

		if (fork_ret == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "ddddd", fd[0], fd[1], kidid,
				      ncperchild, szcharbuf) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			 /*NOTREACHED*/}
#else
			do_child();
#endif
		}

		/* parent */
		sleep(5);
		tst_resm(TINFO, "There are %d children to wait for", kidid);
		for (i = 1; i <= kidid; ++i) {
			wait(&status);
			if (status == 0) {
				tst_resm(TPASS, "child %d exitted successfully",
					 i);
			} else {
				tst_resm(TFAIL, "child %d exitted with bad "
					 "status", i);
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * do_child()
 */
void do_child()
{
	int nread;

	if (close(fd[1])) {
		tst_resm(TINFO, "child %d " "could not close pipe", kidid);
		exit(0);
	}
	nread = safe_read(fd[0], rdbuf, ncperchild);
	if (nread == ncperchild) {
		tst_resm(TINFO, "child %d " "got %d chars", kidid, nread);
		exit(0);
	} else {
		tst_resm(TFAIL, "child %d did not receive expected no of "
			 "characters, got %d characters", kidid, nread);
		exit(1);
	}
}

/*
 * do_child_uclinux() - as above, but mallocs rdbuf first
 */
void do_child_uclinux()
{
	if ((rdbuf = (char *)malloc(szcharbuf)) == (char *)0) {
		tst_brkm(TBROK, cleanup, "malloc of rdbuf failed");
	 /*NOTREACHED*/}

	do_child();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	int i, j;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	numchild = NUMCHILD;
	ncperchild = NCPERCHILD;

	kidid = 0;

	/* allocate read and write buffers */
	szcharbuf = numchild * ncperchild;

	/* make sure pipe write doesn't block */
	if (szcharbuf == PIPE_BUF) {
		/* adjust number of characters per child */
		ncperchild = szcharbuf / numchild;
	}

	if ((wrbuf = (char *)malloc(szcharbuf)) == (char *)0) {
		tst_brkm(TBROK, cleanup, "malloc failed");
	 /*NOTREACHED*/}

	if ((rdbuf = (char *)malloc(szcharbuf)) == (char *)0) {
		tst_brkm(TBROK, cleanup, "malloc of rdbuf failed");
	 /*NOTREACHED*/}

	/* initialize wrbuf */
	j = 0;
	for (i = 0; i < szcharbuf;) {
		wrbuf[i++] = rawchars[j++];
		if (j >= sizeof(rawchars)) {
			j = 0;
		}
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
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
}

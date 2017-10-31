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

char *TCID = "pipe11";
int TST_TOTAL = 1;

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

ssize_t do_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int main(int ac, char **av)
{
	int lc;

	int i;
	int fork_ret, status;
	int written;		/* no of chars read and written */

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "ddddd", &fd[0], &fd[1], &kidid,
			&ncperchild, &szcharbuf);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(pipe(fd));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "pipe creation failed");
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
		}

		if ((fork_ret != 0) && (fork_ret != -1) && (kidid < numchild)) {
			goto refork;
		}

		if (fork_ret == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "ddddd", fd[0], fd[1], kidid,
				      ncperchild, szcharbuf) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
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
				tst_resm(TPASS, "child %d exited successfully",
					 i);
			} else {
				tst_resm(TFAIL, "child %d exited with bad "
					 "status", i);
			}
		}
	}
	cleanup();

	tst_exit();
}

/*
 * do_child()
 */
void do_child(void)
{
	int nread;

	if (close(fd[1])) {
		tst_resm(TINFO, "child %d " "could not close pipe", kidid);
		exit(0);
	}
	nread = do_read(fd[0], rdbuf, ncperchild);
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
void do_child_uclinux(void)
{
	if ((rdbuf = malloc(szcharbuf)) == NULL) {
		tst_brkm(TBROK, cleanup, "malloc of rdbuf failed");
	}

	do_child();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	int i, j;

	tst_sig(FORK, DEF_HANDLER, cleanup);

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

	if ((wrbuf = malloc(szcharbuf)) == NULL) {
		tst_brkm(TBROK, cleanup, "malloc failed");
	}

	if ((rdbuf = malloc(szcharbuf)) == NULL) {
		tst_brkm(TBROK, cleanup, "malloc of rdbuf failed");
	}

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
void cleanup(void)
{

}

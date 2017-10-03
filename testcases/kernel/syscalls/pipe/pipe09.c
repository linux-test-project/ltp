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
 *	pipe09.c
 *
 * DESCRIPTION
 *	Check that two processes can use the same pipe at the same time.
 *
 * ALGORITHM
 *	1. Open a  pipe
 *	2. Fork a child which writes to the pipe
 *	3. Fork another child which writes a different character to the pipe
 *	4. Have the parent read from the pipe
 *	5. It should get the characters from both children.
 *
 * USAGE:  <for command-line>
 *  pipe09 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

#define	PIPEWRTCNT	100	/* must be an even number */

char *TCID = "pipe09";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

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

	int i, red, wtstatus;
	int pipefd[2];		/* fds for pipe read/write */
	char rebuf[BUFSIZ];
	int Acnt = 0, Bcnt = 0;	/* count 'A' and 'B' */
	int fork_1, fork_2;	/* ret values in parent */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(pipe(pipefd));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "pipe() call failed");
			continue;
		}

		if ((fork_1 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #1 failed");
		}

		if (fork_1 == 0) {	/* 1st child */
			if (close(pipefd[0]) != 0) {
				tst_resm(TWARN, "pipefd[0] close failed, "
					 "errno = %d", errno);
				exit(1);
			}

			for (i = 0; i < PIPEWRTCNT / 2; ++i) {
				if (write(pipefd[1], "A", 1) != 1) {
					tst_resm(TWARN, "write to pipe failed");
					exit(1);
				}
			}
			exit(0);
		}

		/* parent */

		SAFE_WAITPID(cleanup, fork_1, &wtstatus, 0);
		if (WIFEXITED(wtstatus) && WEXITSTATUS(wtstatus) != 0) {
			tst_brkm(TBROK, cleanup, "child exited abnormally");
		}

		if ((fork_2 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #2 failed");
		}

		if (fork_2 == 0) {	/* 2nd child */
			if (close(pipefd[0]) != 0) {
				perror("pipefd[0] close failed");
				exit(1);
			}

			for (i = 0; i < PIPEWRTCNT / 2; ++i) {
				if (write(pipefd[1], "B", 1) != 1) {
					perror("write to pipe failed");
					exit(1);
				}
			}
			exit(0);
		}

		/* parent */

		SAFE_WAITPID(cleanup, fork_2, &wtstatus, 0);
		if (WEXITSTATUS(wtstatus) != 0) {
			tst_brkm(TBROK, cleanup, "problem detected in child, "
				 "wait status %d, errno = %d", wtstatus, errno);
		}

		if (close(pipefd[1]) != 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "pipefd[1] close failed");
		}

		while ((red = do_read(pipefd[0], rebuf, 100)) > 0) {
			for (i = 0; i < red; i++) {
				if (rebuf[i] == 'A') {
					Acnt++;
					continue;
				}
				if (rebuf[i] == 'B') {
					Bcnt++;
					continue;
				}
				tst_resm(TFAIL, "got bogus '%c' character",
					 rebuf[i]);
				break;
			}
		}

		if (red == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "reading pipefd pipe failed");
		}

		if (Bcnt == Acnt && Bcnt == (PIPEWRTCNT / 2)) {
			tst_resm(TPASS, "functionality appears to be correct");
		} else {
			tst_resm(TFAIL, "functionality is not correct - Acnt "
				 "= %d, Bcnt = %d", Acnt, Bcnt);
		}

		/* clean up things in case we are looping */
		Acnt = Bcnt = 0;
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
}

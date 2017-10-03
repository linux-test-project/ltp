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
 *	dup204.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of dup2(2).
 *
 * ALGORITHM
 *	attempt to call dup2() on read/write ends of a pipe
 *
 * USAGE:  <for command-line>
 *  dup204 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICTION
 *	NONE
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

void setup();
void cleanup();

char *TCID = "dup204";
int TST_TOTAL = 2;

int fd[2];
int nfd[2];

int main(int ac, char **av)
{
	int lc;
	int i;
	struct stat oldbuf, newbuf;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(dup2(fd[i], nfd[i]));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			SAFE_FSTAT(cleanup, fd[i], &oldbuf);
			SAFE_FSTAT(cleanup, nfd[i], &newbuf);

			if (oldbuf.st_ino != newbuf.st_ino)
				tst_resm(TFAIL, "original and duped "
					 "inodes do not match");
			else
				tst_resm(TPASS, "original and duped "
					 "inodes are the same");

			SAFE_CLOSE(cleanup, TEST_RETURN);
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	fd[0] = -1;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_PIPE(cleanup, fd);
}

void cleanup(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fd); i++) {
		close(fd[i]);
		close(nfd[i]);
	}

	tst_rmdir();
}

/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) Red Hat Inc., 2007
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
 *	sendfile07.c
 *
 * DESCRIPTION
 *	Testcase to test that sendfile(2) system call returns EAGAIN
 *	when passing blocked out_fd. Here out_fd is opend with O_NONBLOCK.
 *
 * ALGORITHM
 *      1. Make sockets with socketpair(&p). Use p[1] as out_fd.
 *      2. Set O_NONBLOCK flag of out_fd on.
 *      3. Write much datum to out_fd till write() returns EAGAIN.
 *      4. Call sendfile with out_fd, and expect EAGAIN.
 *
 * USAGE:  <for command-line>
 *  sendfile07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	12/2007 Copyed from sendfile03.c by Masatake YAMATO
 *
 * RESTRICTIONS
 *	NONE
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include "test.h"

#ifndef OFF_T
#define OFF_T off_t
#endif /* Not def: OFF_T */

TCID_DEFINE(sendfile07);
int TST_TOTAL = 1;

int in_fd, out_fd = 0, ignored_fd = 0;
char in_file[100];

/* To make out_fd overflow, write much chars
 to out_fd. MAX_FILL_DATA_LENGTH defines the `much'. */
#define MAX_FILL_DATA_LENGTH 0xFFFFFFF

void cleanup(void);
void setup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(sendfile(out_fd, in_fd, NULL, 1));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		if (TEST_ERRNO != EAGAIN) {
			tst_resm(TFAIL, "sendfile returned unexpected "
				 "errno, expected: %d, got: %d",
				 EAGAIN, TEST_ERRNO);
		} else {
			tst_resm(TPASS, "sendfile() returned %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	char buf[100];
	int p[2];
	int i, r;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(in_file, "in.%d", getpid());
	if ((in_fd = creat(in_file, 00700)) < 0) {
		tst_brkm(TBROK, cleanup, "creat failed in setup, errno: %d",
			 errno);
	}
	sprintf(buf, "abcdefghijklmnopqrstuvwxyz");
	if (write(in_fd, buf, strlen(buf)) < 0) {
		tst_brkm(TBROK, cleanup, "write failed, errno: %d", errno);
	}
	close(in_fd);
	if ((in_fd = open(in_file, O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed, errno: %d", errno);
	}

	/* Make fulfilled out_fd. */
	if (socketpair(PF_UNIX, SOCK_DGRAM, 0, p) < 0) {
		tst_brkm(TBROK, cleanup, "socketpair failed, errno: %d", errno);
	}

	/* Don't close.
	   You cannot write nothing on out_fd if ignored_fd is closed. */
	ignored_fd = p[0];
	out_fd = p[1];
	if (fcntl(out_fd, F_SETFL, O_WRONLY | O_NONBLOCK) < 0) {
		tst_brkm(TBROK, cleanup, "fcntl failed, errno: %d", errno);
	}

	i = MAX_FILL_DATA_LENGTH;
	while (i > 0) {
		r = write(out_fd, buf, 1);
		if (r < 0) {
			if (errno == EAGAIN) {
				break;
			} else {
				tst_brkm(TBROK, cleanup,
					 "write failed to fill out_fd, errno: %d",
					 errno);
			}
		}
		i--;
	}
	if (i == 0) {
		tst_brkm(TBROK, cleanup,
			 "fail to fill out_fd, write %d bytes but EAGAIN it not returned.",
			 MAX_FILL_DATA_LENGTH);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	if (out_fd)
		close(out_fd);
	if (ignored_fd)
		close(ignored_fd);
	close(in_fd);

	/* delete the test directory created in setup() */
	tst_rmdir();

}

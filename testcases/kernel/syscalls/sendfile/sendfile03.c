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
 *	sendfile03.c
 *
 * DESCRIPTION
 *	Testcase to test that sendfile(2) system call returns appropriete
 *	errnos on error.
 *
 * ALGORITHM
 *	1. Call sendfile(2) with out_fd = -1, and expect EBADF to be returned.
 *	2. Call sendfile(2) with in_fd = -1, and expect EBADF to be returned.
 *	3. Call sendfile(2) with in_fd = out_fd = -1, and expect EBADF.
 *
 * USAGE:  <for command-line>
 *  sendfile03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "test.h"

#define FAILED 1

#ifndef OFF_T
#define OFF_T off_t
#endif /* Not def: OFF_T */

TCID_DEFINE(sendfile03);
int TST_TOTAL = 3;

int in_fd, out_fd;
char in_file[100], out_file[100];

void cleanup(void);
void setup(void);
void setup_func1(void);

struct test_case_t {
	char *desc;
	void (*setupfunc) ();
	int out_fd;
	int in_fd;
	OFF_T *offset;
	int count;
	int exp_errno;
} testcases[] = {
	{
	"Test for EBADF, in_fd = -1", NULL, 8, -1, NULL, 0, EBADF}, {
	"Test for EBADF, out_fd = -1", NULL, -1, 7, NULL, 0, EBADF}, {
	"Test for EBADF, in_fd = out_fd = -1", NULL, -1, -1, NULL,
		    0, EBADF}
};

int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			if (testcases[i].setupfunc != NULL) {
				testcases[i].setupfunc();
			}

			TEST(sendfile(testcases[i].out_fd, testcases[i].in_fd,
				      testcases[i].offset, testcases[i].count));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO != testcases[i].exp_errno) {
				tst_resm(TFAIL, "sendfile returned unexpected "
					 "errno, expected: %d, got: %d",
					 testcases[i].exp_errno, TEST_ERRNO);
			} else {
				tst_resm(TPASS, "sendfile() returned %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
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
	sprintf(out_file, "out.%d", getpid());
	if ((out_fd = open(out_file, O_TRUNC | O_CREAT | O_RDWR, 0777)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed, errno: %d", errno);
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
	close(out_fd);
	close(in_fd);

	/* delete the test directory created in setup() */
	tst_rmdir();

}

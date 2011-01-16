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
 *	fstatfs02.c
 *
 * DESCRIPTION
 *	Testcase to check fstatfs() sets errno correctly.
 *
 * ALGORITHM
 *	1. Pass -1 as the "fd" parameter for fstatfs(), and expect EBADF.
 *	2. Pass an invalid address (outside the address space of the process),
 *	   as the buf paramter of fstatfs(), and expect EFAULT.
 *
 * USAGE:  <for command-line>
 *  fstatfs02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

char *TCID = "fstatfs02";

int exp_enos[] = { EBADF, EFAULT, 0 };

struct statfs buf;

struct test_case_t {
	int fd;
	struct statfs *sbuf;
	int error;
} TC[] = {
	/* EBADF - fd is invalid */
	{
	-1, &buf, EBADF},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* EFAULT - address for buf is invalid */
	{
	1, (void *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int i;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(fstatfs(TC[i].fd, TC[i].sbuf));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();
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

	/* delete the test directory created in setup() */
	tst_rmdir();

}
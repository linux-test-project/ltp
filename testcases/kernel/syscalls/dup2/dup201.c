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
 *	dup201.c
 *
 * DESCRIPTION
 *	Negative tests for dup2() with bad fd (EBADF)
 *
 * ALGORITHM
 * 	Setup:
 *	a.	Setup signal handling.
 *	b.	Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	a.	Loop if the proper options are given.
 *	b.	Loop through the test cases
 * 	c.	Execute dup2() system call
 *	d.	Check return code, if system call failed (return=-1), test
 *		for EBADF.
 *
 * 	Cleanup:
 * 	a.	Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  dup201 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	01/2002 Removed EMFILE test - Paul Larson
 *
 * RESTRICTIONS
 * 	NONE
 */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include "test.h"

void setup(void);
void cleanup(void);

char *TCID = "dup201";
int TST_TOTAL = 4;

int maxfd;
int goodfd = 5;
int badfd = -1;
int mystdout = 0;

struct test_case_t {
	int *ofd;
	int *nfd;
	int error;
	void (*setupfunc) ();
} TC[] = {
	/* First fd argument is less than 0 - EBADF */
	{&badfd, &goodfd, EBADF, NULL},
	    /* First fd argument is getdtablesize() - EBADF */
	{&maxfd, &goodfd, EBADF, NULL},
	    /* Second fd argument is less than 0 - EBADF */
	{&mystdout, &badfd, EBADF, NULL},
	    /* Second fd argument is getdtablesize() - EBADF */
	{&mystdout, &maxfd, EBADF, NULL},
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */

		for (i = 0; i < TST_TOTAL; i++) {

			/* call the test case setup routine if necessary */
			if (TC[i].setupfunc != NULL)
				(*TC[i].setupfunc) ();

			TEST(dup2(*TC[i].ofd, *TC[i].nfd));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS,
					 "failed as expected - errno = %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "failed unexpectedly; "
					 "expected %d: %s", TC[i].error,
					 strerror(TC[i].error));
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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* get some test specific values */
	maxfd = getdtablesize();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
	tst_rmdir();
}

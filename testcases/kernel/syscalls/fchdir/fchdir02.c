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
 *	fchdir02.c
 *
 * DESCRIPTION
 *	fchdir02 - try to cd into a bad directory (bad fd).
 *
 * CALLS
 *	fchdir()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	call fchdir() with an invalid file descriptor
 *	check the errno value
 *	  issue a PASS message if we get EBADF - errno 9
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  fchdir02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

void cleanup(void);
void setup(void);

char *TCID = "fchdir02";
int TST_TOTAL = 1;

int exp_enos[] = { EBADF, 0 };

int main(int ac, char **av)
{
	const int bad_fd = -5;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();		/* global setup */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(fchdir(bad_fd));

		if (TEST_RETURN != -1)
			tst_brkm(TFAIL, cleanup, "call succeeded unexpectedly");

		if (TEST_ERRNO == EBADF)
			tst_resm(TPASS, "failed as expected with EBADF");
		else
			tst_brkm(TFAIL|TTERRNO, cleanup,
			    "call failed unexpectedly");
	}

	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	TEST_EXP_ENOS(exp_enos);
}

void cleanup(void)
{
	tst_rmdir();

	TEST_CLEANUP;
}
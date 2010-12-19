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
 *	open06.c
 *
 * DESCRIPTION
 *	Testcase to check open(2) sets errno to ENXIO correctly.
 *
 * ALGORITHM
 *	Create a named pipe using mknod(2).  Attempt to
 *	open(2) the pipe for writing. The open(2) should
 *	fail with ENXIO.
 *
 * USAGE:  <for command-line>
 *  open06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "open06";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

char fname[100] = "fifo";

int exp_enos[] = { ENXIO, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(open(fname, O_NONBLOCK | O_WRONLY));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "open(2) succeeded unexpectedly");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != ENXIO) {
			tst_resm(TFAIL, "Expected ENXIO got %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "call returned expected ENXIO error");
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

	sprintf(fname, "%s.%d", fname, getpid());

	if (mknod(fname, S_IFIFO|0644, 0) == -1)
		tst_brkm(TBROK, cleanup, "mknod FAILED");
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

	unlink(fname);

	/* delete the test directory created in setup() */
	tst_rmdir();

}

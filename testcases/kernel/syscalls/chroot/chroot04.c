 /*
  *   Copyright (C) Bull S.A. 2001
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
 * 		 chroot04.c
 *
 * DESCRIPTION
 *		 Testcase to check that chroot sets errno to EACCES.
 *
 * ALGORITHM
 *		 As a non-root user attempt to perform chroot() to a directory. The
 *		 chroot() call should fail with EACCES
 *
 * USAGE:  <for command-line>
 *  chroot04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *		 04/2002 Ported by Jacky Malcles
 *
 * RESTRICTIONS
 * 		 Must be run as non-root user.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"
#include <pwd.h>

char *TCID = "chroot04";
int TST_TOTAL = 1;

#define TEST_TMPDIR	"chroot04_tmpdir"

int exp_enos[] = { EACCES, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set up expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(chroot(TEST_TMPDIR));

		if (TEST_RETURN != -1)
			tst_resm(TFAIL, "call succeeded unexpectedly");
		else if (TEST_ERRNO == EACCES)
			tst_resm(TPASS, "got EACCESS as expected");
		else
			tst_resm(TFAIL|TTERRNO, "did not get EACCES as expected");

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

	/*
	 * create a temporary directory
	 */
	if (mkdir(TEST_TMPDIR, 0222) != 0) {
		tst_resm(TBROK, "mkdir(%s) failed", TEST_TMPDIR);
	}

	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, cleanup, "seteuid to nobody failed");
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		        completion or premature exit.
 */
void cleanup()
{
	/* reset the process ID to the saved ID (root) */
	if (setuid(0) == -1) {
		tst_brkm(TBROK|TERRNO, NULL, "setuid(0) failed");
	}
	if (rmdir(TEST_TMPDIR) != 0) {
		tst_brkm(TFAIL|TERRNO, NULL, "rmdir(%s) failed", TEST_TMPDIR);
	}
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

}
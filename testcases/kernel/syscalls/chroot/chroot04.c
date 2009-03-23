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
extern int Tst_count;

char test_dir[100];
int exp_enos[] = { EACCES, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(chroot(test_dir));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
		} else if (TEST_ERRNO != EACCES) {
			tst_resm(TFAIL, "expected EACCES - got %d", TEST_ERRNO);
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TPASS, "expected failure - errno = %d"
				 " : %s", TEST_ERRNO, strerror(TEST_ERRNO));
		}

	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	char *cur_dir = NULL;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	/* get the currect directory name */
	if ((cur_dir = getcwd(cur_dir, 0)) == NULL) {
		tst_brkm(TBROK, cleanup, "Couldn't get current directory name");
	}

	sprintf(test_dir, "%s.%d", cur_dir, getpid());

	/*
	 * create a temporary directory
	 */
	if (mkdir(test_dir, 0222) != 0) {
		tst_resm(TFAIL, "mkdir() failed to create"
			 " a testing directory");
		exit(1);
		/* NOTREACHED */
	}

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
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
		tst_resm(TINFO, "setuid(0) failed");
	}
	if (rmdir(test_dir) != 0) {
		tst_resm(TFAIL, "rmdir() failed to removed"
			 " a testing directory");
		exit(1);
		/* NOTREACHED */
	}
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

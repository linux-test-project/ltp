/*
 *
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
 *	statfs03.c
 *
 * DESCRIPTION
 *	Testcase to check that statfs(2) sets errno to EACCES when
 *	search permission is denied for a component of the path prefix of path.
 *
 * ALGORITHM
 *	 Use a component of the pathname, where search permission
 *	 is denied for a component of the path prefix of path.
 *
 * USAGE:  <for command-line>
 *  statfs03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	05/2002 Ported by Jacky Malcles
 *
 * RESTRICTIONS
 *	NONE
 *
 */
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"
#include <pwd.h>

extern char *TESTDIR;
char *TCID = "statfs03";
int TST_TOTAL = 1;
int fileHandle = 0;
extern int Tst_count;

int exp_enos[] = { EACCES, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char fname[30] = "testfile";
char path[50];
struct statfs buf;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		TEST(statfs(path, &buf));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");

		} else {

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == EACCES) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), exp_enos[0]);
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();
	if (chmod(TESTDIR, S_IRWXU) == -1)
		tst_brkm(TBROK, cleanup, "chmod(%s,700) failed; errno %d: %s",
			 TESTDIR, errno, strerror(errno));

	/* create a test file */
	sprintf(fname, "%s.%d", fname, getpid());
	if (mkdir(fname, 0444) == -1) {
		tst_resm(TFAIL, "creat(2) FAILED to creat temp file");
	} else {
		sprintf(path, "%s/%s", fname, fname);
		if ((fileHandle = creat(path, 0444)) == -1) {
			tst_resm(TFAIL, "creat (2) FAILED to creat temp file");
		}
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
 *	       completion or premature exit.
 */
void cleanup()
{
	/* reset the process ID to the saved ID (root) */
	if (setuid(0) == -1) {
		tst_resm(TINFO, "setuid(0) failed");
	}

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fileHandle);

	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

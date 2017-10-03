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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "safe_macros.h"
#include <pwd.h>

char *TCID = "statfs03";
int TST_TOTAL = 1;
int fileHandle = 0;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char fname[30] = "testfile";
char path[50];
struct statfs buf;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(statfs(path, &buf));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");

		} else {

			if (TEST_ERRNO == EACCES) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), EACCES);
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

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();
	SAFE_CHMOD(cleanup, tst_get_tmpdir(), S_IRWXU);

	/* create a test file */
	sprintf(fname, "%s.%d", fname, getpid());
	if (mkdir(fname, 0444) == -1) {
		tst_resm(TFAIL, "creat(2) FAILED to creat temp file");
	} else {
		sprintf(path, "%s/%s", fname, fname);
		if ((fileHandle = creat(path, 0444)) == -1)
			tst_brkm(TFAIL | TERRNO, cleanup, "creat failed");
	}

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "getpwnam failed");
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO | TERRNO, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
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

	/* delete the test directory created in setup() */
	tst_rmdir();

}

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
 * Test Name: chmod04
 *
 * Test Description:
 *  Verify that, chmod(2) will succeed to change the mode of a directory
 *  and set the sticky bit on it if invoked by non-root (uid != 0) process
 *  with the following constraints,
 *	- the process is the owner of the directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the directory.
 *
 * Expected Result:
 *  chmod() should return value 0 on success and succeeds to set sticky bit
 *  on the specified directory.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  chmod04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be run by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define DIR_MODE 	S_IRWXU | S_IRWXG | S_IRWXO
#define PERMS		01777	/*
				 * Mode permissions of test directory with
				 * sticky bit set.
				 */
#define TESTDIR		"testdir_4"

char *TCID = "chmod04";
int TST_TOTAL = 1;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();
void cleanup();

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat struct. */
	int lc;
	mode_t dir_mode;	/* mode permissions set on testdirectory */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call chmod(2) with mode argument to
		 * set sticky bit on TESTDIR
		 */
		TEST(chmod(TESTDIR, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "chmod(%s, %#o) failed",
				 TESTDIR, PERMS);
			continue;
		}

		/*
		 * Get the file information using
		 * stat(2).
		 */
		if (stat(TESTDIR, &stat_buf) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "stat(2) of %s failed, errno:%d",
				 TESTDIR, TEST_ERRNO);
		}
		dir_mode = stat_buf.st_mode;

		/* Verify STICKY BIT SET on directory */
		if ((dir_mode & PERMS) == PERMS) {
			tst_resm(TPASS, "Functionality of "
				 "chmod(%s, %#o) successful",
				 TESTDIR, PERMS);
		} else {
			tst_resm(TFAIL, "%s: Incorrect modes 0%03o, "
				 "Expected 0%03o",
				 TESTDIR, dir_mode, PERMS);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and cd to it.
 *  Create another test directory under temporary directory.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO | TERRNO, "setuid(%u) failed", ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	/*
	 * Create a test directory under temporary directory with specified
	 * mode permissios.
	 */
	SAFE_MKDIR(cleanup, TESTDIR, DIR_MODE);
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *  Remove the test directory and temporary directory created in setup().
 */
void cleanup(void)
{

	tst_rmdir();

}

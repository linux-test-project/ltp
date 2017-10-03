/*
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
 * Test Name: chmod03
 *
 * Test Description:
 *  Verify that, chmod(2) will succeed to change the mode of a file
 *  and set the sticky bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the file.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the file.
 *
 * Expected Result:
 *  chmod() should return value 0 on success and succeeds to change
 *  the mode of specified file with sticky bit set on it.
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
 *  chmod03 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

#define FILE_MODE       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define PERMS		01777	/*
				 * Mode permissions of test file with sticky
				 * bit set.
				 */
#define TESTFILE	"testfile"

char *TCID = "chmod03";
int TST_TOTAL = 1;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;
	int lc;
	mode_t file_mode;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(chmod(TESTFILE, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "chmod(%s, %#o) failed",
				 TESTFILE, PERMS);
			continue;
		}
		if (stat(TESTFILE, &stat_buf) < 0) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "stat(%s) failed", TESTFILE);
		}
		file_mode = stat_buf.st_mode;

		/* Verify STICKY BIT set on testfile */
		if ((file_mode & PERMS) != PERMS) {
			tst_resm(TFAIL, "%s: Incorrect modes 0%3o, "
				 "Expected 0777", TESTFILE, file_mode);
		} else {
			tst_resm(TPASS, "Functionality of "
				 "chmod(%s, %#o) successful",
				 TESTFILE, PERMS);
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();
	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");
	SAFE_SETUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	/*
	 * Create a test file under temporary directory with specified
	 * mode permissios and set the ownership of the test file to the
	 * uid/gid of guest user.
	 */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TESTFILE, FILE_MODE);
	}

	SAFE_CLOSE(cleanup, fd);
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *  Delete the testfile and temporary directory created in setup().
 */
void cleanup(void)
{

	tst_rmdir();

}

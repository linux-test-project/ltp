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
 * Test Name: fchmod02
 *
 * Test Description:
 *  Verify that, fchmod(2) will succeed to change the mode of a file/directory
 *  set the sticky bit on it if invoked by root (uid = 0) process with
 *  the following constraints,
 *	- the process is not the owner of the file/directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the file/directory.
 *
 * Expected Result:
 *  fchmod() should return value 0 on success and succeeds to set sticky bit
 *  on the specified file.
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
 *  fchmod02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>

#include "test.h"

#define LTPUSER		"nobody"
#define LTPGRP		"users"
#define FILE_MODE 	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define PERMS		01777	/*
				 * Mode permissions of test file with sticky
				 * bit set.
				 */
#define TESTFILE	"testfile"

int fd;				/* file descriptor variable */
char *TCID = "fchmod02";
int TST_TOTAL = 1;

void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call fchmod(2) with specified mode argument
		 * (sticky-bit set) on testfile.
		 */
		TEST(fchmod(fd, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fchmod(%d, %#o) Failed, errno=%d : %s",
				 fd, PERMS, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Get the testfile information using
		 * fstat(2).
		 */
		if (fstat(fd, &stat_buf) < 0) {
			tst_brkm(TFAIL, cleanup, "fstat(2) of %s "
				 "failed, errno:%d", TESTFILE,
				 TEST_ERRNO);
		}

		/* Check for expected mode permissions */
		if ((stat_buf.st_mode & PERMS) == PERMS) {
			tst_resm(TPASS, "Functionality of fchmod(%d, "
				 "%#o) Successful", fd, PERMS);
		} else {
			tst_resm(TFAIL, "%s: Incorrect modes 0%03o, "
				 "Expected 0%03o", TESTFILE,
				 stat_buf.st_mode, PERMS);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory.
 *  Change the ownership of test file to that of "ltpuser1" user.
 */
void setup(void)
{
	struct passwd *ltpuser;	/* password struct for ltpuser1 */
	struct group *ltpgroup;	/* group struct for ltpuser1 */
	gid_t group1_gid;	/* user and process group id's */
	uid_t user1_uid;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get the uid of guest user - ltpuser1 */
	if ((ltpuser = getpwnam(LTPUSER)) == NULL) {
		tst_brkm(TBROK, cleanup, "%s not in /etc/passwd", LTPUSER);
	}
	user1_uid = ltpuser->pw_uid;

	/* Get the group id of guest user - ltpuser1 */
	if ((ltpgroup = getgrnam(LTPGRP)) == NULL) {
		tst_brkm(TBROK, cleanup, "%s not in /etc/group", LTPGRP);
	}
	group1_gid = ltpgroup->gr_gid;

	/*
	 * Create a test file under temporary directory with specified
	 * mode permissios and set the ownership of the test file to the
	 * uid/gid of guest user user1.
	 */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}

	if (chown(TESTFILE, user1_uid, group1_gid) < 0) {
		tst_brkm(TBROK, cleanup, "chown(2) of %s failed", TESTFILE);
	}

	/* Set the effective gid of the process to that of user */
	if (setgid(group1_gid) < 0) {
		tst_brkm(TBROK, cleanup, "setgid(2) to %d failed", group1_gid);
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the testfile created in the setup.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	/* Close the testfile created in the setup() */
	if (close(fd) == -1) {
		tst_brkm(TBROK, NULL, "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	}

	tst_rmdir();

}

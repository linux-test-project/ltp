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
 * Test Name: fchmod05
 *
 * Test Description:
 *  Verify that, fchmod(2) will succeed to change the mode of a directory
 *  but fails to set the setgid bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is not equal to the group ID of the directory.
 *
 * Expected Result:
 *  fchmod() should return value 0 on success and though succeeds to change
 *  the mode of a directory but fails to set setgid bit on it.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  fchmod05 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#include "test.h"

#define MODE_RWX	(S_IRWXU | S_IRWXG | S_IRWXO)
#define PERMS		043777
#define TESTDIR		"testdir"

int fd;				/* file descriptor for test directory */
char *TCID = "fchmod05";
int TST_TOTAL = 1;

void setup();			/* Main setup function for test */
void cleanup();			/* Main cleanup function for test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat struct */
	int lc;
	mode_t dir_mode;	/* mode permissions set on test directory */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call fchmod(2) with mode argument
		 * to set setgid bit on TESTDIR.
		 */

		TEST(fchmod(fd, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fchmod(%d, %#o) Failed, errno=%d : %s",
				 fd, PERMS, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Get the directory information using
		 * fstat(2).
		 */
		if (fstat(fd, &stat_buf) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "fstat(2) of %s failed, errno:%d",
				 TESTDIR, TEST_ERRNO);
		}
		dir_mode = stat_buf.st_mode;
		if ((PERMS & ~S_ISGID) != dir_mode) {
			tst_resm(TFAIL, "%s: Incorrect modes 0%03o, "
				 "Expected 0%03o",
				 TESTDIR, dir_mode, PERMS & ~S_ISGID);
		} else {
			tst_resm(TPASS, "Functionality of fchmod(%d, "
				 "%#o) successful", fd,
				 PERMS & ~S_ISGID);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and cd to it.
 *  Create a test directory under temporary directory.
 *  Invoke setuid to root program to modify group ownership
 *  on test directory.
 *  Open the test directory for reading.
 */
void setup(void)
{
	struct passwd *nobody_u;
	struct group *bin_group;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	nobody_u = getpwnam("nobody");
	if (!nobody_u)
		tst_brkm(TBROK, cleanup,
			 "Couldn't find uid of nobody: %s", strerror(errno));

	bin_group = getgrnam("bin");
	if (!bin_group)
		tst_brkm(TBROK, cleanup,
			 "Couldn't find gid of bin: %s", strerror(errno));

	/*
	 * Create a test directory under temporary directory with specified
	 * mode permissions and change the gid of test directory to that of
	 * guest user.
	 */
	if (mkdir(TESTDIR, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) of %s failed", TESTDIR);
	}

	if (setgroups(1, &nobody_u->pw_gid) == -1)
		tst_brkm(TBROK, cleanup,
			 "Couldn't change supplementary group Id: %s",
			 strerror(errno));

	if (chown(TESTDIR, nobody_u->pw_uid, bin_group->gr_gid) == -1)
		tst_brkm(TBROK, cleanup, "Couldn't change owner of testdir: %s",
			 strerror(errno));

	/* change to nobody:nobody */
	if (setegid(nobody_u->pw_gid) == -1 || seteuid(nobody_u->pw_uid) == -1)
		tst_brkm(TBROK, cleanup, "Couldn't switch to nobody:nobody: %s",
			 strerror(errno));

	/* Open the test directory for reading */
	fd = open(TESTDIR, O_RDONLY);
	if (fd == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDONLY) failed, errno=%d : %s",
			 TESTDIR, errno, strerror(errno));
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *  Close the test directory opened in the setup().
 *  Remove the test directory and temporary directory created in
 *  in the setup().
 */
void cleanup(void)
{

	/* Close the test directory opened in the setup() */
	if (close(fd) == -1) {
		tst_brkm(TBROK, NULL,
			 "close(%s) Failed, errno=%d : %s",
			 TESTDIR, errno, strerror(errno));
	}

	setegid(0);
	seteuid(0);

	tst_rmdir();

}

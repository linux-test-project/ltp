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
 * Test Name: chmod05
 *
 * Test Description:
//wjh
//This test seems to be invalid see comments below
//The man page for chmod doesn't seem to indicate that the setgid bit can
//only be set by root
 *  Verify that, chmod(2) will succeed to change the mode of a directory
 *  but fails to set the setgid bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the directory.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is not equal to the group ID of the directory.
 *
 * Expected Result:
 *  chmod() should return value 0 on success and though succeeds to change
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
 *  chmod05 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	05/2002 Fixes by wjhuie and Ferhan Shareef
 *                  changed conditional to check for valid tests only and
 *                  in a more logically understandable way
 *	07/2002 Additional fixes to #defines to allow comparisions to work.
 *		-Robbie Williamson
 *
 * RESTRICTIONS:
 *  This test should be run by root.
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

#define DEBUG 0

#define MODE_RWX	(mode_t)(S_IRWXU | S_IRWXG | S_IRWXO)
#define DIR_MODE	(mode_t)(S_ISVTX | S_ISGID | S_IFDIR)
#define PERMS		(mode_t)(MODE_RWX | DIR_MODE)
#define TESTDIR		"testdir"

char *TCID = "chmod05";
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

		TEST(chmod(TESTDIR, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "chmod(%s, %#o) failed",
				 TESTDIR, PERMS);
			continue;
		}
		/*
		 * Get the directory information using
		 * stat(2).
		 */
		if (stat(TESTDIR, &stat_buf) < 0) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "stat(%s) failed", TESTDIR);
		}
		dir_mode = stat_buf.st_mode;
#if DEBUG
		printf("DIR_MODE = 0%03o\n", DIR_MODE);
		printf("MODE_RWX = 0%03o\n", MODE_RWX);
		printf("PERMS = 0%03o\n", PERMS);
		printf("dir_mode = 0%03o\n", dir_mode);
#endif
		if ((PERMS & ~S_ISGID) != dir_mode)
			tst_resm(TFAIL, "%s: Incorrect modes 0%03o, "
				 "Expected 0%03o", TESTDIR, dir_mode,
				 PERMS & ~S_ISGID);
		else
			tst_resm(TPASS,
				 "Functionality of chmod(%s, %#o) successful",
					 TESTDIR, PERMS);
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and cd to it.
 *  Create a test directory under temporary directory.
//wjh we are root so do we really need this kluged helper program?
 *  Invoke setuid to root program to modify group ownership
 *  on test directory.
 */
void setup(void)
{
	struct passwd *nobody_u;
	struct group *bin_group;

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	nobody_u = getpwnam("nobody");
	if (nobody_u == NULL)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "getpwnam(\"nobody\") failed");

	bin_group = getgrnam("bin");
	if (bin_group == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "getgrnam(\"bin\") failed");

	/*
	 * Create a test directory under temporary directory with specified
	 * mode permissions and change the gid of test directory to nobody's
	 * gid.
	 */
	if (mkdir(TESTDIR, MODE_RWX) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "mkdir(%s) failed", TESTDIR);

	if (setgroups(1, &nobody_u->pw_gid) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "setgroups to nobody's gid failed");

	if (chown(TESTDIR, nobody_u->pw_uid, bin_group->gr_gid) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "chowning testdir to nobody:bin failed");

	/* change to nobody:nobody */
	if (setegid(nobody_u->pw_gid) == -1 || seteuid(nobody_u->pw_uid) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't switch to nobody:nobody");
}

void cleanup(void)
{
	if (setegid(0) == -1)
		tst_resm(TWARN | TERRNO, "setegid(0) failed");
	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();
}

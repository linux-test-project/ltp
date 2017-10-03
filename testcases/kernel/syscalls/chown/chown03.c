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
 * Test Name: chown03
 *
 * Test Description:
 *  Verify that, chown(2) succeeds to change the group of a file specified
 *  by path when called by non-root user with the following constraints,
 *	- euid of the process is equal to the owner of the file.
 *	- the intended gid is either egid, or one of the supplementary gids
 *	  of the process.
 *  Also, verify that chown() clears the setuid/setgid bits set on the file.
 *
 * Expected Result:
 *  chown(2) should return 0 and the ownership set on the file should match
 *  the numeric values contained in owner and group respectively.
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
 *  chown03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FILE_MODE	(S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define NEW_PERMS	(S_IFREG|S_IRWXU|S_IRWXG|S_ISUID|S_ISGID)
#define TESTFILE	"testfile"

TCID_DEFINE(chown03);
int TST_TOTAL = 1;		/* Total number of test conditions */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;
	uid_t user_id;		/* Owner id of the test file. */
	gid_t group_id;		/* Group id of the test file. */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		UID16_CHECK((user_id = geteuid()), "chown", cleanup)
		GID16_CHECK((group_id = getegid()), "chown", cleanup)

		TEST(CHOWN(cleanup, TESTFILE, -1, group_id));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "chown(%s, ..) failed",
				 TESTFILE);
			continue;
		}

		if (stat(TESTFILE, &stat_buf) == -1)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "stat failed");

		if (stat_buf.st_uid != user_id ||
		    stat_buf.st_gid != group_id)
			tst_resm(TFAIL, "%s: Incorrect ownership"
				 "set to %d %d, Expected %d %d",
				 TESTFILE, stat_buf.st_uid,
				 stat_buf.st_gid, user_id, group_id);

		if (stat_buf.st_mode !=
		    (NEW_PERMS & ~(S_ISUID | S_ISGID)))
			tst_resm(TFAIL, "%s: incorrect mode permissions"
				 " %#o, Expected %#o", TESTFILE,
				 stat_buf.st_mode,
				 NEW_PERMS & ~(S_ISUID | S_ISGID));
		else
			tst_resm(TPASS, "chown(%s, ..) was successful",
				 TESTFILE);
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 *  Change the group ownership on testfile.
 */
void setup(void)
{
	int fd;			/* file handler for testfile */

	TEST_PAUSE;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getpwnam(\"nobody\") failed");
	SAFE_SETEGID(NULL, ltpuser->pw_gid);
	SAFE_SETEUID(NULL, ltpuser->pw_uid);

	/* Create a test file under temporary directory */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed", TESTFILE,
			 FILE_MODE);

	SAFE_SETEUID(cleanup, 0);

	SAFE_FCHOWN(cleanup, fd, -1, 0);

	SAFE_FCHMOD(cleanup, fd, NEW_PERMS);

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	SAFE_CLOSE(cleanup, fd);
}

void cleanup(void)
{
	if (setegid(0) == -1)
		tst_resm(TWARN | TERRNO, "setegid(0) failed");
	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();

}

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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: lchown01
 *
 * Test Description:
 *  Verify that, lchown(2) succeeds to change the owner and group of a file
 *  specified by path to any numeric owner(uid)/group(gid) values when invoked
 *  by super-user.
 *
 * Expected Result:
 *  lchown(2) should return 0 and the ownership set on the file should match
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
 *  lchown01 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 *	11/2010 Code cleanup by Cyril Hrubis chrubis@suse.cz
 *
 * RESTRICTIONS:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

#define FILE_MODE	(S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define TESTFILE	"testfile"
#define SFILE		"slink_file"

char *TCID = "lchown01";
int TST_TOTAL = 5;

struct test_case_t {
	char *desc;
	uid_t user_id;
	gid_t group_id;
};

static struct test_case_t test_cases[] = {
	{"Change Owner/Group ids", 700, 701},
	{"Change Owner id only",   702,  -1},
	{"Change Owner/Group ids", 703, 701},
	{"Change Group id only",    -1, 704},
	{"Change Group/Group ids", 703, 705},
	{"Change none",             -1,  -1},
	{NULL,                       0,   0}
};

void setup(void);
void cleanup(void);

int main(int argc, char *argv[])
{
	struct stat stat_buf;
	int lc;
	char *msg;
	int i;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; test_cases[i].desc != NULL; i++) {
			uid_t user_id = test_cases[i].user_id;
			gid_t group_id = test_cases[i].group_id;
			char *test_desc = test_cases[i].desc;

			/*
			 * Call lchown(2) with different user id and
			 * group id (numeric values) to set it on
			 * symlink of testfile.
			 */
			TEST(lchown(SFILE, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "lchown() Fails to %s, errno %d",
					 test_desc, TEST_ERRNO);
				continue;
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Get the testfile information using
				 * lstat(2).
				 */
				if (lstat(SFILE, &stat_buf) < 0) {
					tst_brkm(TFAIL, cleanup, "lstat(2) "
					         "%s failed, errno %d",
						 SFILE, TEST_ERRNO);
				}
				if (user_id == -1) {
					if (i > 0)
						user_id = test_cases[i-1].user_id;
					else
						user_id = geteuid();
				}
				if (group_id == -1) {
					if (i > 0)
						group_id = test_cases[i-1].group_id;
					else
						group_id = getegid();
				}

				/*
				 * Check for expected Ownership ids
				 * set on testfile.
				 */
				if ((stat_buf.st_uid != user_id) ||
				    (stat_buf.st_gid != group_id)) {
					tst_resm(TFAIL,
						 "%s: incorrect ownership set, "
						 "Expected %d %d", SFILE,
						 user_id, group_id);
				} else {
					tst_resm(TPASS, "lchown() succeeds to "
						 "%s of %s", test_desc, SFILE);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Create a temporary directory and change directory to it.
 *	     Create a test file under temporary directory and close it
 *	     Create a symlink of testfile under temporary directory.
 */
void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;
	tst_tmpdir();

	if ((fd = open(TESTFILE, O_RDWR|O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup, "open failed");
	}
	if (close(fd) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "close failed");
	}

	if (symlink(TESTFILE, SFILE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "symlink failed");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *	       Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
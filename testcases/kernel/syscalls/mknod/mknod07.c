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
 * Test Name: mknod07
 *
 * Test Description:
 * Verify that,
 *   1) mknod(2) returns -1 and sets errno to EPERM if the process id of
 *	the caller is not super-user.
 *   2) mknod(2) returns -1 and sets errno to EACCES if parent directory
 *	does not allow  write  permission  to  the process.
 *
 * Expected Result:
 *  mknod() should fail with return value -1 and set expected errno.
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
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  mknod07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by non-super-user only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX	S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO
#define NEWMODE		S_IFIFO | S_IRUSR | S_IRGRP | S_IROTH
#define SOCKET_MODE	S_IFSOCK| S_IRWXU | S_IRWXG | S_IRWXO
#define DIR_TEMP	"testdir_1"

void setup2();			/* setup function to test mknod for EACCES */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	int mode;
	int exp_errno;
	void (*setupfunc)(void);
} test_cases[] = {
	{ "tnode_1", SOCKET_MODE, EACCES, NULL },
	{ "tnode_2", NEWMODE, EACCES, setup2 },
};

char *TCID = "mknod07";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
int exp_enos[] = { EPERM, EACCES, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* setup function for the tests */
void cleanup();			/* cleanup function for the tests */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *node_name;	/* ptr. for node name created */
	int i;		/* counter to test different test conditions */
	int mode;		/* creation mode for the node created */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			node_name = test_cases[i].pathname;
			mode = test_cases[i].mode;

			TEST(mknod(node_name, mode, 0));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "mknod succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == test_cases[i].exp_errno)
				tst_resm(TPASS|TTERRNO,
				    "mknod failed as expected");
			else
				tst_resm(TFAIL|TTERRNO,
				    "mknod failed unexpectedly; expected: "
				    "%d - %s", test_cases[i].exp_errno,
				    strerror(test_cases[i].exp_errno));
		}

	}

	cleanup();
	tst_exit();
}

void setup()
{
	int i;

	tst_require_root(NULL);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (seteuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "setuid failed");

	TEST_PAUSE;

	tst_tmpdir();

	if (mkdir(DIR_TEMP, MODE_RWX) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir failed");

	if (chmod(DIR_TEMP, MODE_RWX) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod failed");

	if (chdir(DIR_TEMP) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chdir failed");

	for (i = 0; i < TST_TOTAL; i++)
		if (test_cases[i].setupfunc != NULL)
			test_cases[i].setupfunc();
}

void setup2()
{
	if (chmod(".", NEWMODE) < 0)
		tst_brkm(TBROK, cleanup, "chmod failed");
}

void cleanup()
{
	TEST_CLEANUP;

	if (seteuid(0) == -1)
		tst_resm(TBROK, "seteuid(0) failed");

	tst_rmdir();

}
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
 * Test Name: mknod06
 *
 * Test Description:
 * Verify that,
 *   1) mknod(2) returns -1 and sets errno to EEXIST if specified path
 *	already exists.
 *   2) mknod(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   3) mknod(2) returns -1 and sets errno to ENOENT if the directory
 *	component in pathname does not exist.
 *   4) mknod(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component was too long.
 *   5) mknod(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
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
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  mknod06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be executed by super-user (root) only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "test.h"

#define MODE_RWX		S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO

int setup1();			/* setup function to test mknod for EEXIST */
int setup3();			/* setup function to test mknod for ENOTDIR */
int longpath_setup();		/* setup function to test mknod for ENAMETOOLONG */
int no_setup();			/* simply returns 0 to the caller */
char Longpathname[PATH_MAX + 2];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{"tnode_1", "Specified node already exists", EEXIST, setup1}, {
	NULL, "Invalid address", EFAULT, no_setup}, {
	"testdir_2/tnode_2", "Non-existent file", ENOENT, no_setup}, {
	"", "Pathname is empty", ENOENT, no_setup}, {
	Longpathname, "Pathname too long", ENAMETOOLONG, longpath_setup}, {
	"tnode/tnode_3", "Path contains regular file", ENOTDIR, setup3}, {
	NULL, NULL, 0, no_setup}
};

char *TCID = "mknod06";
int TST_TOTAL = ARRAY_SIZE(Test_cases);

void setup();			/* setup function for the tests */
void cleanup();			/* cleanup function for the tests */

int main(int ac, char **av)
{
	int lc;
	char *node_name;	/* ptr. for node name created */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */

	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * Invoke setup function to call individual test setup functions
	 * for the test which run as root/super-user.
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			node_name = Test_cases[ind].pathname;
			test_desc = Test_cases[ind].desc;

			/*
			 * Call mknod(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(mknod(node_name, MODE_RWX, 0));

			/* Check return code from mknod(2) */
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL,
					 "mknod() returned %ld, expected "
					 "-1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
				continue;
			}

			if (TEST_ERRNO == Test_cases[ind].exp_errno) {
				tst_resm(TPASS, "mknod() fails, %s, errno:%d",
					 test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "mknod() fails, %s, errno:%d, "
					 "expected errno:%d", test_desc,
					 TEST_ERRNO, Test_cases[ind].exp_errno);
			}
		}

	}

	/*
	 * Invoke cleanup() to delete the test directories created
	 * in the setup().
	 */
	cleanup();

	tst_exit();
}

/*
 * setup(void) - performs all ONE TIME setup for this test.
 * 	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory used to hold test directories and nodes
 *	created and change the directory to it.
 *	Invoke individual test setup functions according to the order
 *	set in struct. definition.
 */
void setup(void)
{
	int ind;

	tst_require_root();

	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		if (!Test_cases[ind].pathname)
			Test_cases[ind].pathname = tst_get_bad_addr(cleanup);
		Test_cases[ind].setupfunc();
	}
}

/*
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *		Hence, this function just returns 0.
 */
int no_setup(void)
{
	return 0;
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *		      the MAX. length of PATH_MAX.
 *   This function retruns 0.
 */
int longpath_setup(void)
{
	int ind;		/* counter variable */

	for (ind = 0; ind <= (PATH_MAX + 1); ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;
}

/*
 * setup1() - setup function for a test condition for which mknod(2)
 *	      returns -1 and sets errno to EEXIST.
 *  This function creates a node using mknod(2) and tries to create
 *  same node in the test and fails with above errno.
 *  This function returns 0.
 */
int setup1(void)
{
	/* Create a node using mknod */
	if (mknod("tnode_1", MODE_RWX, 0) < 0) {
		tst_brkm(TBROK, cleanup, "Fails to create node in setup1()");
	}

	return 0;
}

/*
 * setup3() - setup function for a test condition for which mknod(2)
 *	      returns -1 and sets errno to ENOTDIR.
 *  This function creates a node under temporary directory and the
 *  test attempts to create another node under under this node and fails
 *  with ENOTDIR as the node created here is a regular file.
 *  This function returns 0.
 */
int setup3(void)
{
	/* Create a node using mknod */
	if (mknod("tnode", MODE_RWX, 0) < 0) {
		tst_brkm(TBROK, cleanup, "Fails to create node in setup3()");
	}

	return 0;
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
 */
void cleanup(void)
{

	tst_rmdir();

}

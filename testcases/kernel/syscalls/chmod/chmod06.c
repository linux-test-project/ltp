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
 * Test Name: chmod06
 *
 * Test Description:
 *   Verify that,
 *   1) chmod(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) chmod(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   3) chmod(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) chmod(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) chmod(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) chmod(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists.
 *
 * Expected Result:
 *  chmod() should fail with return value -1 and set expected errno.
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
 *  chmod06 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define TEST_FILE3	"t_file/tfile_3"

int setup1();		/* setup function to test chmod for EPERM */
int setup2();		/* setup function to test chmod for EACCES */
int setup3();		/* setup function to test chmod for ENOTDIR */
int longpath_setup();	/* setup function to test chmod for ENAMETOOLONG */

char *test_home;		/* variable to hold TESTHOME env. */
char Longpathname[PATH_MAX + 2];
char High_address_node[64];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	mode_t mode;
	int exp_errno;
	int (*setupfunc) ();
} test_cases[] = {
	/* Process not owner/root */
	{ TEST_FILE1, FILE_MODE, EPERM, setup1},
	/* No search permissions to process */
	{ TEST_FILE2, FILE_MODE, EACCES, setup2},
	/* Address beyond address space */
	{ High_address_node, FILE_MODE, EFAULT, NULL },
	/* Negative address #1 */
	{ (char *)-1, FILE_MODE, EFAULT, NULL },
	/* Negative address #2 */
	{ (char *)-2, FILE_MODE, EFAULT, NULL },
	/* Pathname too long. */
	{ Longpathname, FILE_MODE, ENAMETOOLONG, longpath_setup},
	/* Pathname empty. */
	{ "", FILE_MODE, ENOENT, NULL },
	/* Pathname contains a regular file. */
	{ TEST_FILE3, FILE_MODE, ENOTDIR, setup3 },
};

char *TCID = "chmod06";
int TST_TOTAL = sizeof(test_cases) / sizeof(*test_cases);
int exp_enos[] = { EPERM, EACCES, EFAULT, ENAMETOOLONG, ENOENT, ENOTDIR, 0 };

char *bad_addr = 0;

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	char *msg;
	char *file_name;
	int i;
	mode_t mode;
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			file_name = test_cases[i].pathname;
			mode = test_cases[i].mode;

			if (file_name == High_address_node)
				file_name = get_high_address();
			if (i < 2) {
				ltpuser = getpwnam(nobody_uid);
				if (ltpuser == NULL)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "getpwnam failed");
				if (seteuid(ltpuser->pw_uid) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "seteuid failed");
			}
			if (i >= 2)
				seteuid(0);

			TEST(chmod(file_name, mode));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "chmod succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == test_cases[i].exp_errno)
				tst_resm(TPASS|TTERRNO,
				    "chmod failed as expected");
			else
				tst_resm(TFAIL|TTERRNO,
				    "chmod failed unexpectedly; "
				    "expected %d - %s",
				    test_cases[i].exp_errno,
				    strerror(test_cases[i].exp_errno));
		}

	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup(void) - performs all ONE TIME setup for this test.
 * 	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 *	Invoke iividual test setup functions according to the order
 *	set in struct. definition.
 */
void setup()
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	test_home = get_current_dir_name();

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX|MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");
	test_cases[3].pathname = bad_addr;

	for (i = 0; i < TST_TOTAL; i++)
		if (test_cases[i].setupfunc != NULL)
			test_cases[i].setupfunc();
}

/*
 * int
 * setup1() - setup function for a test condition for which chmod(2)
 *	      returns -1 and sets errno to EPERM.
 *
 *  Create a testfile under temporary directory and invoke setuid to root
 *  program to change the ownership of testfile to that of "ltpuser2" user.
 *
 */
int setup1()
{
	int fd;

	/* open/creat a test file and close it */
	fd = open(TEST_FILE1, O_RDWR | O_CREAT, 0666);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed",
			 TEST_FILE1);

	if (fchown(fd, 0, 0) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "fchown(%s) failed",
			TEST_FILE1);

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "close(%s) failed",
			 TEST_FILE1);

	return 0;
}

/*
 * int
 * setup2() - setup function for a test condition for which mknod(2)
 *	      returns -1 and sets errno to EACCES.
 *  Create a test directory under temporary directory and create a test file
 *  under this directory with mode "0666" permissions.
 *  Modify the mode permissions on test directory such that process will not
 *  have search permissions on test directory.
 *
 *  The function returns 0.
 */
int setup2()
{
	int fd;			/* file handle for testfile */

	/* Creat a test directory and a file under it */
	if (mkdir(DIR_TEMP, MODE_RWX) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s) failed", DIR_TEMP);

	fd = open(TEST_FILE2, O_RDWR | O_CREAT, 0666);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
		    "open(%s, O_RDWR|O_CREAT, 0666) failed", TEST_FILE2);

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", TEST_FILE2);

	if (chmod(DIR_TEMP, FILE_MODE) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s) failed", DIR_TEMP);

	return 0;
}

/*
 * int
 * setup3() - setup function for a test condition for which chmod(2)
 *	     returns -1 and sets errno to ENOTDIR.
 *
 *  Create a test file under temporary directory so that test tries to
 *  change mode of a testfile "tfile_3" under "t_file" which happens to be
 *  another regular file.
 */
int setup3()
{
	int fd;

	/* Create a test file under temporary directory and close it */
	fd = open("t_file", O_RDWR|O_CREAT, MODE_RWX);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open(t_file) failed");

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(t_file) failed");

	return 0;
}

int longpath_setup()
{
	int i;

	for (i = 0; i <= (PATH_MAX + 1); i++)
		Longpathname[i] = 'a';
	return 0;
}

void cleanup()
{
	TEST_CLEANUP;

	if (chmod(DIR_TEMP, MODE_RWX) == -1)
		tst_resm(TBROK|TERRNO, "chmod(%s) failed", DIR_TEMP);

	tst_rmdir();
}
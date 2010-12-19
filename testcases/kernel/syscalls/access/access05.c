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
 * Test Name: access05
 *
 * Test Description:
 *  Verify that,
 *   1. access() fails with -1 return value and sets errno to EACCES
 *      if the permission bits of the file mode do not permit the
 *	 requested (Read/Write/Execute) access.
 *   2. access() fails with -1 return value and sets errno to EINVAL
 *	if the specified access mode argument is invalid.
 *   3. access() fails with -1 return value and sets errno to EFAULT
 *	if the pathname points outside allocate address space for the
 *	process.
 *   4. access() fails with -1 return value and sets errno to ENOENT
 *	if the specified file doesn't exist (or pathname is NULL).
 *   5. access() fails with -1 return value and sets errno to ENAMETOOLONG
 *      if the pathname size is > PATH_MAX characters.
 *
 * Expected Result:
 *  access() should fail with return value -1 and set expected errno.
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
 *  access05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define INV_OK		-1
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define TEST_FILE4	"test_file4"

#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

void setup1();			/* setup() to test access() for EACCES */
void setup2();			/* setup() to test access() for EACCES */
void setup3();			/* setup() to test access() for EACCES */
void setup4();			/* setup() to test access() for EINVAL */
void longpath_setup();		/* setup function to test access() for ENAMETOOLONG */

#if !defined(UCLINUX)
char high_address_node[64];
#endif

char Longpathname[PATH_MAX + 2];

struct test_case_t {		/* test case structure */
	char *pathname;
	int a_mode;
	int exp_errno;
	void (*setupfunc)(void);
} test_cases[] = {
	{ TEST_FILE1, R_OK, EACCES, setup1 },
	{ TEST_FILE2, W_OK, EACCES, setup2 },
	{ TEST_FILE3, X_OK, EACCES, setup3 },
	{ TEST_FILE4, INV_OK, EINVAL, setup4 },
#if !defined(UCLINUX)
	{ (char *)-1, R_OK, EFAULT, NULL },
	{ high_address_node, R_OK, EFAULT, NULL },
#endif
	{ "", W_OK, ENOENT, NULL },
	{ Longpathname, R_OK, ENAMETOOLONG, longpath_setup },
};

char *TCID = "access05";	/* Test program identifier.    */
int TST_TOTAL = sizeof(test_cases) / sizeof(*test_cases);
int exp_enos[] = { EACCES, EFAULT, EINVAL, ENOENT, ENAMETOOLONG, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

char *bad_addr = 0;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *file_name;	/* name of the testfile */
	int access_mode;	/* specified access mode for testfile */
	int i;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			file_name = test_cases[i].pathname;
			access_mode = test_cases[i].a_mode;

#if !defined(UCLINUX)
			if (file_name == high_address_node)
				file_name = get_high_address();
#endif

			/*
			 * Call access(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(access(file_name, access_mode));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL,
				    "access(%s, %#o) succeeded unexpectedly",
				    file_name, access_mode);
				continue;
			}

			if (TEST_ERRNO == test_cases[i].exp_errno)
				tst_resm(TPASS|TTERRNO,
				    "access failed as expected");
			else
				tst_resm(TFAIL|TTERRNO,
				    "access failed unexpectedly; expected: "
				    "%d - %s",
				    test_cases[i].exp_errno,
				    strerror(test_cases[i].exp_errno));
		}
	}

	cleanup();

	tst_exit();

}

void setup()
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "setuid failed");

	TEST_PAUSE;

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX|MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, NULL, "mmap failed");
	test_cases[5].pathname = bad_addr;
#endif

	tst_tmpdir();

	for (i = 0; i < TST_TOTAL; i++)
		if (test_cases[i].setupfunc != NULL)
			test_cases[i].setupfunc();
}

void setup_file(const char *file, mode_t perms)
{
	int fd;		/* file handle for testfile */

	if ((fd = open(file, O_RDWR | O_CREAT, FILE_MODE)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 file, FILE_MODE);

	if (fchmod(fd, perms) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s, %#o) failed",
			 file, perms);
	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", file);
}

/*
 * setup1() - Setup function to test access() for return value -1
 *	      and errno EACCES when read access denied for specified
 *	      testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny read access permissions on testfile.
 *   This function returns 0.
 */
void setup1()
{
	setup_file(TEST_FILE1, 0333);
}

/*
 * setup2() - Setup function to test access() for return value -1 and
 *	      errno EACCES when write access denied on testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny write access permissions on testfile.
 *   This function returns 0.
 */
void setup2()
{
	setup_file(TEST_FILE2, 0555);
}

/*
 * setup3() - Setup function to test access() for return value -1 and
 *	      errno EACCES when execute access denied on testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny search access permissions on testfile.
 *   This function returns 0.
 */
void setup3()
{
	setup_file(TEST_FILE3, 0666);
}

/*
 * setup4() - Setup function to test access() for return value -1
 *	      and errno EINVAL when specified access mode argument is
 *	      invalid.
 *
 *   Creat/open a testfile and close it.
 *   This function returns 0.
 */
void setup4()
{
	setup_file(TEST_FILE4, FILE_MODE);
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *		      the MAX. length of PATH_MAX.
 */
void longpath_setup()
{
	int i;

	for (i = 0; i <= (PATH_MAX + 1); i++)
		Longpathname[i] = 'a';
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Delete the test directory/file and temporary directory
	 * created in the setup.
	 */
	tst_rmdir();

}
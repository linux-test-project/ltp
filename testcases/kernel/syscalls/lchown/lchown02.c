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
 * Test Name: lchown02
 *
 * Test Description:
 *   Verify that,
 *   1) lchown(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) lchown(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   3) lchown(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) lchown(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) lchown(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) lchown(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists.
 *
 * Expected Result:
 *  lchown() should fail with return value -1 and set expected errno.
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
 *  lchown02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
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
 *      11/2010 Rewritten by Cyril Hrubis chrubis@suse.cz
 *
 * RESTRICTIONS:
 *
 */

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

#define TEST_USER       "nobody"
#define MODE_RWX	S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define SFILE1		"sfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define SFILE2		"testdir_1/sfile_2"
#define TFILE3          "t_file"
#define SFILE3		"t_file/sfile"

char *TCID = "lchown02";
int TST_TOTAL = 7;

static void setup_eperm(int pos);
static void setup_eacces(int pos);
static void setup_enotdir(int pos);
static void setup_longpath(int pos);
static void setup_efault(int pos);
static void setup_highaddress(int pos);

static char path[PATH_MAX + 2];

struct test_case_t {
	char *pathname;
	char *desc;
	int exp_errno;
	void (*setup)(int pos);
};

static struct test_case_t test_cases[] = {
	{SFILE1, "Process is not owner/root",    EPERM,   setup_eperm},
	{SFILE2, "Search permission denied",     EACCES,  setup_eacces},
	{NULL,   "Address beyond address space", EFAULT,  setup_highaddress},
	{NULL,   "Unaccessible address space",   EFAULT,  setup_efault},
	{path,   "Pathname too long",            ENAMETOOLONG, setup_longpath},
	{SFILE3, "Path contains regular file",   ENOTDIR, setup_enotdir},
	{"",     "Pathname is empty",            ENOENT,  NULL},
	{NULL,   NULL,                           0,       NULL}
};

static struct passwd *ltpuser;

static int exp_enos[] =
	{EPERM, EACCES, EFAULT, ENAMETOOLONG, ENOENT, ENOTDIR, 0};

void setup(void);
void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	char *msg;
	uid_t user_id;
	gid_t group_id;
	int i;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();
	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	user_id = geteuid();
	group_id = getegid();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		for (i = 0; test_cases[i].desc != NULL; i++) {
			char *file_name = test_cases[i].pathname;
			char *test_desc = test_cases[i].desc;

			/*
			 * Call lchown(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(lchown(file_name, user_id, group_id));

			/* Check return code from lchown(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == test_cases[i].exp_errno) {
					tst_resm(TPASS,
						 "lchown(2) fails, %s, errno:%d",
						 test_desc, TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "lchown(2) fails, %s, "
						 "errno:%d, expected errno:%d",
						 test_desc, TEST_ERRNO,
						 test_cases[i].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "lchown(2) returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 test_cases[i].exp_errno);
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 *	Invoke individual test setup functions.
 */
void setup(void)
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;

	/* change euid and gid to nobody */
	ltpuser = getpwnam(TEST_USER);

	if (ltpuser == NULL)
		tst_brkm(TBROK, cleanup, "getpwnam failed");

	if (setgid(ltpuser->pw_uid) == -1)
		tst_resm(TBROK | TERRNO, "setgid failed");

	tst_tmpdir();

	for (i = 0; test_cases[i].desc != NULL; i++)
		if (test_cases[i].setup != NULL)
			test_cases[i].setup(i);
}

/*
 * setup_eperm() - setup function for a test condition for which lchown(2)
 *	           returns -1 and sets errno to EPERM.
 *
 * Create test file and symlink with uid 0.
 */
void setup_eperm(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a testfile */
	if ((fd = open(TEST_FILE1, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close failed");

	/* become root once more */
	if (seteuid(0) == -1)
		tst_resm(TBROK|TERRNO, "setuid(0) failed");

	/* create symling to testfile */
	if (symlink(TEST_FILE1, SFILE1) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "symlink failed");

	/* back to the user nobody */
	if (seteuid(ltpuser->pw_uid) == -1)
		tst_resm(TBROK|TERRNO, "seteuid(%d) failed", ltpuser->pw_uid);
}

/*
 * setup_eaccess() - setup function for a test condition for which lchown(2)
 *	             returns -1 and sets errno to EACCES.
 *
 *  Create a test directory under temporary directory and create a test file
 *  under this directory with mode "0666" permissions.
 *  Modify the mode permissions on test directory such that process will not
 *  have search permissions on test directory.
 */
void setup_eacces(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a test directory */
	if (mkdir(DIR_TEMP, MODE_RWX) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir failed");

	/* create a file under test directory */
	if ((fd = open(TEST_FILE2, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close failed");

	/* create a symlink of testfile */
	if (symlink(TEST_FILE2, SFILE2) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "symlink(2) %s to %s failed",
			 TEST_FILE2, SFILE2);
	}

	/* modify mode permissions on test directory */
	if (chmod(DIR_TEMP, FILE_MODE) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "chmod(2) %s failed",
		         DIR_TEMP);
	}
}

/*
 * setup_efault() -- setup for a test condition where lchown(2) returns -1 and
 *                   sets errno to EFAULT.
 *
 * Create "bad address" by explicitly mmaping anonymous page that may not be
 * accesed (see PROT_NONE).
 */
static void setup_efault(int pos)
{
	char *bad_addr = 0;

	bad_addr = mmap(NULL, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, -1, 0);

	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");

	test_cases[pos].pathname = bad_addr;
}

/*
 * setup_efault() -- setup for a test condition where lchown(2) returns -1 and
 *                   sets errno to EFAULT.
 *
 * Use ltp function get_high_address() to compute high address.
 */
static void setup_highaddress(int pos)
{
	test_cases[pos].pathname = get_high_address();
}

/*
 * setup_enotdir() - setup function for a test condition for which chown(2)
 *	             returns -1 and sets errno to ENOTDIR.
 *
 * Create a regular file "t_file" to call lchown(2) on "t_file/sfile" later.
 */
void setup_enotdir(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a testfile under temporary directory */
	if ((fd = open(TFILE3, O_RDWR | O_CREAT, MODE_RWX)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "open(2) %s failed",
		         TFILE3);
	}

	if (close(fd) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "close(2) %s failed",
		         TFILE3);
	}
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *                    the length of PATH_MAX.
 */
void setup_longpath(int pos)
{
	memset(test_cases[pos].pathname, 'a', PATH_MAX + 1);
	test_cases[pos].pathname[PATH_MAX + 1] = '\0';
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 */
void cleanup(void)
{
	TEST_CLEANUP;

	/* become root again */
	if (seteuid(0) == -1) {
		tst_resm(TINFO|TERRNO,
		         "seteuid(2) failed to set the effective uid to 0");
	}

	tst_rmdir();
}

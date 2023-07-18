/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      11/2010 Rewritten by Cyril Hrubis chrubis@suse.cz
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
#include "safe_macros.h"

/*
 * Don't forget to remove USE_LEGACY_COMPAT_16_H from Makefile after
 * rewriting all tests to the new API.
 */
#include "compat_16.h"

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

TCID_DEFINE(lchown02);
int TST_TOTAL = 7;

static void setup(void);
static void cleanup(void);
static void setup_eperm(int pos);
static void setup_eacces(int pos);
static void setup_enotdir(int pos);
static void setup_longpath(int pos);
static void setup_efault(int pos);

static char path[PATH_MAX + 2];

struct test_case_t {
	char *pathname;
	char *desc;
	int exp_errno;
	void (*setup) (int pos);
};

static struct test_case_t test_cases[] = {
	{SFILE1, "Process is not owner/root", EPERM, setup_eperm},
	{SFILE2, "Search permission denied", EACCES, setup_eacces},
	{NULL, "Unaccessible address space", EFAULT, setup_efault},
	{path, "Pathname too long", ENAMETOOLONG, setup_longpath},
	{SFILE3, "Path contains regular file", ENOTDIR, setup_enotdir},
	{"", "Pathname is empty", ENOENT, NULL},
	{NULL, NULL, 0, NULL}
};

static struct passwd *ltpuser;

int main(int argc, char *argv[])
{
	int lc;
	uid_t user_id;
	gid_t group_id;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	user_id = geteuid();
	UID16_CHECK(user_id, lchown, cleanup);
	group_id = getegid();
	GID16_CHECK(group_id, lchown, cleanup);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; test_cases[i].desc != NULL; i++) {
			char *file_name = test_cases[i].pathname;
			char *test_desc = test_cases[i].desc;

			/*
			 * Call lchown(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(LCHOWN(cleanup, file_name, user_id, group_id));

			/* Check return code from lchown(2) */
			if (TEST_RETURN == -1) {
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

static void setup(void)
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

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
static void setup_eperm(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a testfile */
	if ((fd = open(TEST_FILE1, O_RDWR | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "open failed");

	SAFE_CLOSE(cleanup, fd);

	/* become root once more */
	if (seteuid(0) == -1)
		tst_resm(TBROK | TERRNO, "setuid(0) failed");

	/* create symling to testfile */
	SAFE_SYMLINK(cleanup, TEST_FILE1, SFILE1);

	/* back to the user nobody */
	if (seteuid(ltpuser->pw_uid) == -1)
		tst_resm(TBROK | TERRNO, "seteuid(%d) failed", ltpuser->pw_uid);
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
static void setup_eacces(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a test directory */
	SAFE_MKDIR(cleanup, DIR_TEMP, MODE_RWX);

	/* create a file under test directory */
	if ((fd = open(TEST_FILE2, O_RDWR | O_CREAT, 0666)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "open failed");

	SAFE_CLOSE(cleanup, fd);

	/* create a symlink of testfile */
	SAFE_SYMLINK(cleanup, TEST_FILE2, SFILE2);

	/* modify mode permissions on test directory */
	SAFE_CHMOD(cleanup, DIR_TEMP, FILE_MODE);
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
	test_cases[pos].pathname = tst_get_bad_addr(cleanup);
}

/*
 * setup_enotdir() - setup function for a test condition for which chown(2)
 *	             returns -1 and sets errno to ENOTDIR.
 *
 * Create a regular file "t_file" to call lchown(2) on "t_file/sfile" later.
 */
static void setup_enotdir(int pos LTP_ATTRIBUTE_UNUSED)
{
	int fd;

	/* create a testfile under temporary directory */
	if ((fd = open(TFILE3, O_RDWR | O_CREAT, MODE_RWX)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "open(2) %s failed", TFILE3);
	}

	SAFE_CLOSE(cleanup, fd);
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *                    the length of PATH_MAX.
 */
static void setup_longpath(int pos)
{
	memset(test_cases[pos].pathname, 'a', PATH_MAX + 1);
	test_cases[pos].pathname[PATH_MAX + 1] = '\0';
}

static void cleanup(void)
{
	if (seteuid(0) == -1) {
		tst_resm(TINFO | TERRNO,
			 "seteuid(2) failed to set the effective uid to 0");
	}

	tst_rmdir();
}

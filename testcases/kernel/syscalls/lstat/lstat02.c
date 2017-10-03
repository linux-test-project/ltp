/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
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
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * Test Description:
 *   Verify that,
 *   1) lstat(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   2) lstat(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists or empty string.
 *   3) lstat(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) lstat(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) lstat(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) lstat(2) returns -1 and sets errno to ELOOP if the pathname has too
 *	many symbolic links encountered while traversing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define MODE_RWX	S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TEST_DIR	"test_dir"
#define TEST_EACCES	TEST_DIR"/test_eacces"
#define TEST_ENOENT	""
#define TEST_ENOTDIR	"test_file/test_enotdir"
#define TEST_ELOOP	"/test_eloop"

static char longpathname[PATH_MAX + 2];
static char elooppathname[sizeof(TEST_ELOOP) * 43] = ".";

#if !defined(UCLINUX)
static void bad_addr_setup(int);
static void high_address_setup(int);
#endif

static struct test_case_t {
	char *pathname;
	int exp_errno;
	void (*setup) ();
} test_cases[] = {
	{TEST_EACCES, EACCES, NULL},
	{TEST_ENOENT, ENOENT, NULL},
#if !defined(UCLINUX)
	{NULL, EFAULT, bad_addr_setup},
	{NULL, EFAULT, high_address_setup},
#endif
	{longpathname, ENAMETOOLONG, NULL},
	{TEST_ENOTDIR, ENOTDIR, NULL},
	{elooppathname, ELOOP, NULL},
};

char *TCID = "lstat02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void lstat_verify(int);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			lstat_verify(i);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;
	struct passwd *ltpuser;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_MKDIR(cleanup, TEST_DIR, MODE_RWX);
	SAFE_TOUCH(cleanup, TEST_EACCES, 0666, NULL);
	SAFE_CHMOD(cleanup, TEST_DIR, FILE_MODE);

	SAFE_TOUCH(cleanup, "test_file", MODE_RWX, NULL);

	memset(longpathname, 'a', PATH_MAX+1);

	SAFE_MKDIR(cleanup, "test_eloop", MODE_RWX);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	for (i = 0; i < 43; i++)
		strcat(elooppathname, TEST_ELOOP);
}

#if !defined(UCLINUX)
static void bad_addr_setup(int i)
{
	test_cases[i].pathname = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
					   MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

static void high_address_setup(int i)
{
	test_cases[i].pathname = (char *)get_high_address();
}
#endif

static void lstat_verify(int i)
{
	struct stat stat_buf;

	if (test_cases[i].setup != NULL)
		test_cases[i].setup(i);

	TEST(lstat(test_cases[i].pathname, &stat_buf));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "lstat() returned %ld, expected -1, errno=%d",
			 TEST_RETURN, test_cases[i].exp_errno);
		return;
	}

	if (TEST_ERRNO == test_cases[i].exp_errno) {
		tst_resm(TPASS | TTERRNO, "lstat() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "lstat() failed unexpectedly; expected: %d - %s",
			 test_cases[i].exp_errno,
			 strerror(test_cases[i].exp_errno));
	}
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_resm(TINFO | TERRNO, "Failet to seteuid(0) before cleanup");

	tst_rmdir();
}

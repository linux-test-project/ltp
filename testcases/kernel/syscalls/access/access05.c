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
 *   6. access() fails with -1 return value and sets errno to ENOTDIR
 *      if a component used as a directory in pathname is not a directory.
 *   7. access() fails with -1 return value and sets errno to ELOOP
 *      if too many symbolic links were encountered in resolving pathname.
 *
 *   07/2001 Ported by Wayne Boyer
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
#include "safe_macros.h"

#define INV_OK		-1
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define TEST_FILE4	"test_file4"
#define TEST_FILE5	"test_file5/test_file5"
#define TEST_FILE6	"test_file6"


#if !defined(UCLINUX)
static char high_address_node[64];
#endif

static char longpathname[PATH_MAX + 2];

static struct test_case_t {
	char *pathname;
	int a_mode;
	int exp_errno;
} test_cases[] = {
	{TEST_FILE1, R_OK, EACCES},
	{TEST_FILE2, W_OK, EACCES},
	{TEST_FILE3, X_OK, EACCES},
	{TEST_FILE4, INV_OK, EINVAL},
#if !defined(UCLINUX)
	{(char *)-1, R_OK, EFAULT},
	{high_address_node, R_OK, EFAULT},
#endif
	{"", W_OK, ENOENT},
	{longpathname, R_OK, ENAMETOOLONG},
	{TEST_FILE5, R_OK, ENOTDIR},
	{TEST_FILE6, R_OK, ELOOP},
};

char *TCID = "access05";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static int exp_enos[] = { EACCES, EFAULT, EINVAL, ENOENT, ENAMETOOLONG,
			  ENOTDIR, ELOOP, 0 };

static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void setup(void);
static void access_verify(int i);
static void cleanup(void);

static char *bad_addr;

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int i;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			access_verify(i);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	tst_require_root(NULL);

	ltpuser = SAFE_GETPWNAM(cleanup, nobody_uid);
	SAFE_SETUID(cleanup, ltpuser->pw_uid);
	TEST_PAUSE;

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, NULL, "mmap failed");
	test_cases[4].pathname = bad_addr;

	test_cases[5].pathname = get_high_address();
#endif

	tst_tmpdir();

	/*
	 * create TEST_FILE1 to test R_OK EACCESS
	 */
	fd = SAFE_CREAT(cleanup, TEST_FILE1, 0333);
	SAFE_CLOSE(cleanup, fd);

	/*
	 * create TEST_FILE2 to test W_OK EACCESS
	 */
	fd = SAFE_CREAT(cleanup, TEST_FILE2, 0555);
	SAFE_CLOSE(cleanup, fd);

	/*
	 * create TEST_FILE3 to test X_OK EACCESS
	 */
	fd = SAFE_CREAT(cleanup, TEST_FILE3, 0666);
	SAFE_CLOSE(cleanup, fd);

	/*
	 * create TEST_FILE4 to test EINVAL
	 */
	fd = SAFE_CREAT(cleanup, TEST_FILE4, 0333);
	SAFE_CLOSE(cleanup, fd);

	/*
	 *setup to create a node with a name length exceeding
	 *the MAX length of PATH_MAX.
	 */
	memset(longpathname, 'a', sizeof(longpathname) - 1);

	/* create test_file5 for test ENOTDIR. */
	SAFE_TOUCH(cleanup, "test_file5", 0644, NULL);

	/*
	 * create two symbolic links who point to each other for
	 * test ELOOP.
	 */
	SAFE_SYMLINK(cleanup, "test_file6", "test_file7");
	SAFE_SYMLINK(cleanup, "test_file7", "test_file6");
}

static void access_verify(int i)
{
	char *file_name;
	int access_mode;

	file_name = test_cases[i].pathname;
	access_mode = test_cases[i].a_mode;

	TEST(access(file_name, access_mode));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "access(%s, %#o) succeeded unexpectedly",
			 file_name, access_mode);
		return;
	}

	if (TEST_ERRNO == test_cases[i].exp_errno) {
		tst_resm(TPASS | TTERRNO, "access failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "access failed unexpectedly; expected: "
			 "%d - %s", test_cases[i].exp_errno,
			 strerror(test_cases[i].exp_errno));
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}

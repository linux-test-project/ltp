/*
 *
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
 * Test Description :
 *   Verify that,
 *   1) readlink(2) returns -1 and sets errno to EACCES if search/write
 *	permission is denied in the directory where the symbolic link
 *	resides.
 *   2) readlink(2) returns -1 and sets errno to EINVAL if the buffer size
 *	is not positive.
 *   3) readlink(2) returns -1 and sets errno to EINVAL if the specified
 *	file is not a symbolic link file.
 *   4) readlink(2) returns -1 and sets errno to ENAMETOOLONG if the
 *	pathname component of symbolic link is too long (ie, > PATH_MAX).
 *   5) readlink(2) returns -1 and sets errno to ENOENT if the component of
 *	symbolic link points to an empty string.
 *   6) readlink(2) returns -1 and sets errno to ENOTDIR if a component of
 *	the path prefix is not a directory.
 *   7) readlink(2) returns -1 and sets errno to ELOOP if too many symbolic
 *	links were encountered in translating the pathname.
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define MODE_RWX	(S_IRWXU | S_IRWXG | S_IRWXO)
#define FILE_MODE	(S_IRUSR | S_IRGRP | S_IROTH)
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"testdir_1/tfile_1"
#define SYM_FILE1	"testdir_1/sfile_1"
#define TEST_FILE2	"tfile_2"
#define SYM_FILE2	"sfile_2"
#define TEST_FILE3	"tfile_3"
#define SYM_FILE3	"tfile_3/sfile_3"
#define ELOOPFILE	"/test_eloop"
#define MAX_SIZE	256

static char longpathname[PATH_MAX + 2];
static char elooppathname[sizeof(ELOOPFILE) * 43] = ".";

static struct test_case_t {
	char *link;
	size_t buf_size;
	int exp_errno;
} test_cases[] = {
	{SYM_FILE1, 1, EACCES},
	    /* Don't test with bufsize -1, since this cause a fortify-check-fail when
	       using glibc and -D_FORITY_SOURCE=2

	       Discussion: http://lkml.org/lkml/2008/10/23/229
	       Conclusion: Only test with 0 as non-positive bufsize.

	       { SYM_FILE2, -1, EINVAL, NULL },
	     */
	{SYM_FILE2, 0, EINVAL},
	{TEST_FILE2, 1, EINVAL},
	{longpathname, 1, ENAMETOOLONG},
	{"", 1, ENOENT},
	{SYM_FILE3, 1, ENOTDIR},
	{elooppathname, 1, ELOOP},
};

static void setup(void);
static void readlink_verify(struct test_case_t *);
static void cleanup(void);

char *TCID = "readlink03";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			readlink_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	struct passwd *ltpuser;
	int i;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_MKDIR(cleanup, DIR_TEMP, MODE_RWX);
	SAFE_TOUCH(cleanup, TEST_FILE1, 0666, NULL);
	SAFE_SYMLINK(cleanup, TEST_FILE1, SYM_FILE1);
	SAFE_CHMOD(cleanup, DIR_TEMP, FILE_MODE);

	SAFE_TOUCH(cleanup, TEST_FILE2, 0666, NULL);
	SAFE_SYMLINK(cleanup, TEST_FILE2, SYM_FILE2);

	memset(longpathname, 'a', PATH_MAX + 1);

	SAFE_TOUCH(cleanup, TEST_FILE3, 0666, NULL);

	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limit in kernel is hardwired to 40.
	 */
	SAFE_MKDIR(cleanup, "test_eloop", MODE_RWX);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(elooppathname, ELOOPFILE);
}

void readlink_verify(struct test_case_t *tc)
{
	char buffer[MAX_SIZE];

	if (tc->buf_size == 1)
		tc->buf_size = sizeof(buffer);

	TEST(readlink(tc->link, buffer, tc->buf_size));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "readlink() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "readlink() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"readlink() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
		if (tc->exp_errno == ENOENT && TEST_ERRNO == EINVAL) {
			tst_resm(TWARN | TTERRNO,
				"It may be a Kernel Bug, see the patch:"
				"http://git.kernel.org/linus/1fa1e7f6");
		}
	}
}

void cleanup(void)
{
	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	tst_rmdir();
}

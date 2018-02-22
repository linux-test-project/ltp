/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 John George
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
 *   Test Description:
 *     Verify that,
 *     1) truncate(2) returns -1 and sets errno to EACCES if search/write
 *        permission denied for the process on the component of the path prefix
 *        or named file.
 *     2) truncate(2) returns -1 and sets errno to ENOTDIR if the component of
 *        the path prefix is not a directory.
 *     3) truncate(2) returns -1 and sets errno to EFAULT if pathname points
 *        outside user's accessible address space.
 *     4) truncate(2) returns -1 and sets errno to ENAMETOOLONG if the component
 *        of a pathname exceeded 255 characters or entire pathname exceeds 1023
 *        characters.
 *     5) truncate(2) returns -1 and sets errno to ENOENT if the named file
 *        does not exist.
 *     6) truncate(2) returns -1 and sets errno to EISDIR if the named file
 *        is a directory.
 *     7) truncate(2) returns -1 and sets errno to EFBIG if the argument length
 *        is larger than the maximum file size.
 *     8) truncate(2) returns -1 and sets errno to ELOOP if too many symbolic
 *        links were encountered in translating the pathname.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/resource.h>

#include "test.h"
#include "safe_macros.h"

#define TEST_FILE1	"testfile"
#define TEST_FILE2	"t_file/testfile"
#define TEST_FILE3	"testfile3"
#define TEST_SYM1	"testsymlink1"
#define TEST_SYM2	"testsymlink2"
#define TEST_DIR1	"testdir"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define NEW_MODE	S_IRUSR | S_IRGRP | S_IROTH
#define DIR_MODE	S_IRWXU
#define TRUNC_LEN	256
#define MAX_FSIZE	(16*1024*1024)

static char long_pathname[PATH_MAX + 2];

static struct test_case_t {
	char *pathname;
	off_t length;
	int exp_errno;
} test_cases[] = {
	{ TEST_FILE1, TRUNC_LEN, EACCES },
	{ TEST_FILE2, TRUNC_LEN, ENOTDIR },
	{ NULL, TRUNC_LEN, EFAULT },
	{ long_pathname, TRUNC_LEN, ENAMETOOLONG },
	{ "", TRUNC_LEN, ENOENT },
	{ TEST_DIR1, TRUNC_LEN, EISDIR },
	{ TEST_FILE3, MAX_FSIZE*2, EFBIG },
	{ TEST_SYM1, TRUNC_LEN, ELOOP }
};

static void setup(void);
static void cleanup(void);
static void truncate_verify(struct test_case_t *);

char *TCID = "truncate03";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			truncate_verify(&test_cases[i]);

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	struct passwd *ltpuser;
	struct rlimit rlim;
	sigset_t signalset;
	int n;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TEST_FILE1, NEW_MODE, NULL);

	SAFE_TOUCH(cleanup, "t_file", FILE_MODE, NULL);

	memset(long_pathname, 'a', PATH_MAX + 1);

	SAFE_MKDIR(cleanup, TEST_DIR1, DIR_MODE);

	SAFE_TOUCH(cleanup, TEST_FILE3, FILE_MODE, NULL);

	SAFE_SYMLINK(cleanup, TEST_SYM1, TEST_SYM2);
	SAFE_SYMLINK(cleanup, TEST_SYM2, TEST_SYM1);

	rlim.rlim_cur = MAX_FSIZE;
	rlim.rlim_max = MAX_FSIZE;
	SAFE_SETRLIMIT(cleanup, RLIMIT_FSIZE, &rlim);

	sigemptyset(&signalset);
	sigaddset(&signalset, SIGXFSZ);
	TEST(sigprocmask(SIG_BLOCK, &signalset, NULL));
	if (TEST_RETURN != 0)
		tst_brkm(TBROK | TTERRNO, cleanup, "sigprocmask");

	for (n = 0; n < TST_TOTAL; n++) {
		if (!test_cases[n].pathname)
			test_cases[n].pathname = tst_get_bad_addr(cleanup);
	}

}

void truncate_verify(struct test_case_t *tc)
{
	TEST(truncate(tc->pathname, tc->length));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "truncate() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "truncate() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"truncate() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}
}

void cleanup(void)
{
	tst_rmdir();
}

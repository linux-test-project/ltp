// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2022
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 John George
 */

/*\
 * [Description]
 *
 * Verify that:
 *
 * - truncate(2) returns -1 and sets errno to EACCES if search/write
 *   permission denied for the process on the component of the path prefix
 *   or named file.
 * - truncate(2) returns -1 and sets errno to ENOTDIR if the component of
 *   the path prefix is not a directory.
 * - truncate(2) returns -1 and sets errno to EFAULT if pathname points
 *   outside user's accessible address space.
 * - truncate(2) returns -1 and sets errno to ENAMETOOLONG if the component
 *   of a pathname exceeded 255 characters or entire pathname exceeds 1023
 *   characters.
 * - truncate(2) returns -1 and sets errno to ENOENT if the named file
 *    does not exist.
 * - truncate(2) returns -1 and sets errno to EISDIR if the named file
 *   is a directory.
 * - truncate(2) returns -1 and sets errno to EFBIG if the argument length
 *   is larger than the maximum file size.
 * - truncate(2) returns -1 and sets errno to ELOOP if too many symbolic
 *   links were encountered in translating the pathname.
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

#include "tst_test.h"

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

static void setup(void)
{
	struct passwd *ltpuser;
	struct rlimit rlim = {
		.rlim_cur = MAX_FSIZE,
		.rlim_max = MAX_FSIZE,
	};
	sigset_t signalset;
	unsigned int n;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);

	SAFE_TOUCH(TEST_FILE1, NEW_MODE, NULL);

	SAFE_TOUCH("t_file", FILE_MODE, NULL);

	memset(long_pathname, 'a', PATH_MAX + 1);

	SAFE_MKDIR(TEST_DIR1, DIR_MODE);

	SAFE_TOUCH(TEST_FILE3, FILE_MODE, NULL);

	SAFE_SYMLINK(TEST_SYM1, TEST_SYM2);
	SAFE_SYMLINK(TEST_SYM2, TEST_SYM1);

	SAFE_SETRLIMIT(RLIMIT_FSIZE, &rlim);

	SAFE_SIGEMPTYSET(&signalset);
	SAFE_SIGADDSET(&signalset, SIGXFSZ);
	SAFE_SIGPROCMASK(SIG_BLOCK, &signalset, NULL);

	for (n = 0; n < ARRAY_SIZE(test_cases); n++) {
		if (!test_cases[n].pathname)
			test_cases[n].pathname = tst_get_bad_addr(NULL);
	}

}

static void verify_truncate(unsigned int n)
{
	struct test_case_t *tc = &test_cases[n];

	TST_EXP_FAIL(truncate(tc->pathname, tc->length), tc->exp_errno);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.tcnt = ARRAY_SIZE(test_cases),
	.test = verify_truncate,
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*\
 * [Description]
 *
 * Test basic error handling of statx syscall:
 *
 * - EBADF - Bad file descriptor
 * - EFAULT - Bad address
 * - EINVAL - Invalid argument
 * - ENOENT - No such file or directory
 * - ENOTDIR - Not a directory
 * - ENAMETOOLONG - Filename too long
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_get_bad_addr.h"
#include "lapi/stat.h"

#define TESTFILE "test_file"
#define MODE 0644

static char long_pathname[257];

static char *test_fname = TESTFILE;
static char *efault_fname;
static char *empty_fname = "";
static char *etoolong_fname = long_pathname;

static struct test_case {
	uint32_t dfd;
	char **filename;
	uint32_t flag;
	uint32_t mask;
	int32_t errnum;
} tcases[] = {
	{.dfd = -1, .filename = &test_fname, .flag = 0,
	 .mask = 0, .errnum = EBADF},

	{.dfd = AT_FDCWD, .filename = &efault_fname, .flag = 0,
	 .mask = 0, .errnum = EFAULT},

	{.dfd = AT_FDCWD, .filename = &test_fname, .flag = -1,
	 .mask = 0, .errnum = EINVAL},

	{.dfd = AT_FDCWD, .filename = &test_fname, .flag = 0,
	 .mask = -1, .errnum = EINVAL},

	{.dfd = AT_FDCWD, .filename = &empty_fname, .flag = 0,
	 .mask = 0, .errnum = ENOENT},

	{.dfd = 1, .filename = &test_fname, .flag = 0,
	 .mask = 0, .errnum = ENOTDIR},

	{.dfd = AT_FDCWD, .filename = &etoolong_fname, .flag = 0,
	 .mask = 0, .errnum = ENAMETOOLONG},
};

static void run_test(unsigned int i)
{
	struct statx buf;
	struct test_case *tc = &tcases[i];

	TEST(statx(tc->dfd, *(tc->filename), tc->flag,
		   tc->mask, &buf));

	if (TST_RET != -1) {
		tst_res(TFAIL, "statx() returned with %ld", TST_RET);
		return;
	}

	if (tc->errnum == TST_ERR) {
		tst_res(TPASS | TTERRNO, "statx() failed with");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"statx() should fail with %s", tst_strerrno(tc->errnum));
}

static void setup(void)
{
	int file_fd;

	file_fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, MODE);
	SAFE_CLOSE(file_fd);

	memset(long_pathname, '@', sizeof(long_pathname));
	long_pathname[sizeof(long_pathname) - 1] = 0;

	efault_fname = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run_test,
	.setup = setup,
	.min_kver = "4.11",
	.needs_tmpdir = 1,
};

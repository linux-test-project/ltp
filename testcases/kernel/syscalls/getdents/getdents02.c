/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	         written by Wayne Boyer
 * Copyright (c) 2013 Markos Chandras
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
 * Test Description:
 *  Verify that,
 *   1. getdents() fails with -1 return value and sets errno to EBADF
 *      if file descriptor fd is invalid.
 *   2. getdents() fails with -1 return value and sets errno to EINVAL
 *      if result buffer is too small.
 *   3. getdents() fails with -1 return value and sets errno to ENOTDIR
 *      if file descriptor does not refer to a directory.
 *   4. getdents() fails with -1 return value and sets errno to ENOENT
 *      if there is no such directory.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test.h"
#include "getdents.h"
#include "safe_macros.h"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define TEST_DIR	"test_dir"

static void cleanup(void);
static void setup(void);
static void print_test_result(int err, int exp_errno);

char *TCID = "getdents02";

static void test_ebadf(void);
static void test_einval(void);
static void test_enotdir(void);
static void test_enoent(void);

static void (*testfunc[])(void) = { test_ebadf, test_einval,
				    test_enotdir, test_enoent };

int TST_TOTAL = ARRAY_SIZE(testfunc);

static int longsyscall;

option_t options[] = {
		/* -l long option. Tests getdents64 */
		{"l", &longsyscall, NULL},
		{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -l      Test the getdents64 system call\n");
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, options, &help);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;
}

static void print_test_result(int err, int exp_errno)
{
	if (err == 0) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
	} else if  (err == exp_errno) {
		tst_resm(TPASS, "getdents failed as expected: %s",
			 strerror(err));
	} else if (err == ENOSYS) {
		tst_resm(TCONF, "syscall not implemented");
	} else {
		tst_resm(TFAIL, "getdents failed unexpectedly: %s",
			 strerror(err));
	}
}

static void test_ebadf(void)
{
	int fd = -5;
	struct linux_dirent64 dirp64;
	struct linux_dirent dirp;

	if (longsyscall)
		getdents64(fd, &dirp64, sizeof(dirp64));
	else
		getdents(fd, &dirp, sizeof(dirp));

	print_test_result(errno, EBADF);
}

static void test_einval(void)
{
	int fd;
	char buf[1];

	fd = SAFE_OPEN(cleanup, ".", O_RDONLY);

	/* Pass one byte long buffer. The result should be EINVAL */
	if (longsyscall)
		getdents64(fd, (void *)buf, sizeof(buf));
	else
		getdents(fd, (void *)buf, sizeof(buf));

	print_test_result(errno, EINVAL);

	SAFE_CLOSE(cleanup, fd);
}

static void test_enotdir(void)
{
	int fd;
	struct linux_dirent64 dir64;
	struct linux_dirent dir;

	fd = SAFE_OPEN(cleanup, "test", O_CREAT | O_RDWR, 0644);

	if (longsyscall)
		getdents64(fd, &dir64, sizeof(dir64));
	else
		getdents(fd, &dir, sizeof(dir));

	print_test_result(errno, ENOTDIR);

	SAFE_CLOSE(cleanup, fd);
}

static void test_enoent(void)
{
	int fd;
	struct linux_dirent64 dir64;
	struct linux_dirent dir;

	SAFE_MKDIR(cleanup, TEST_DIR, DIR_MODE);

	fd = SAFE_OPEN(cleanup, TEST_DIR, O_DIRECTORY);
	SAFE_RMDIR(cleanup, TEST_DIR);

	if (longsyscall)
		getdents64(fd, &dir64, sizeof(dir64));
	else
		getdents(fd, &dir, sizeof(dir));

	print_test_result(errno, ENOENT);

	SAFE_CLOSE(cleanup, fd);
}

static void cleanup(void)
{
	tst_rmdir();
}

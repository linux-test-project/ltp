/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

/*
 * DESCRIPTION
 *  Basic test for fcntl(2) using F_SETLEASE & F_WRLCK argument.
 *  "A write lease may be placed on a file only if there are
 *  no other open file descriptors for the file."
 */

#include <errno.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void verify_fcntl(int);
static void cleanup(void);

#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)

static int fd1;
static int fd2;
static long type;

static struct test_case_t {
	int fd1_flag;
	int fd2_flag;
} test_cases[] = {
	{O_RDONLY, O_RDONLY},
	{O_RDONLY, O_WRONLY},
	{O_RDONLY, O_RDWR},
	{O_WRONLY, O_RDONLY},
	{O_WRONLY, O_WRONLY},
	{O_WRONLY, O_RDWR},
	{O_RDWR, O_RDONLY},
	{O_RDWR, O_WRONLY},
	{O_RDWR, O_RDWR},
};

char *TCID = "fcntl32";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int lc;
	int tc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (tc = 0; tc < TST_TOTAL; tc++)
			verify_fcntl(tc);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	tst_tmpdir();

	switch ((type = tst_fs_type(cleanup, "."))) {
	case TST_NFS_MAGIC:
	case TST_RAMFS_MAGIC:
	case TST_TMPFS_MAGIC:
		tst_brkm(TCONF, cleanup,
			 "Cannot do fcntl(F_SETLEASE, F_WRLCK) "
			 "on %s filesystem",
			 tst_fs_type_name(type));
	default:
		break;
	}

	SAFE_TOUCH(cleanup, "file", FILE_MODE, NULL);
}

static void verify_fcntl(int i)
{
	fd1 = SAFE_OPEN(cleanup, "file", test_cases[i].fd1_flag);
	fd2 = SAFE_OPEN(cleanup, "file", test_cases[i].fd2_flag);

	TEST(fcntl(fd1, F_SETLEASE, F_WRLCK));

	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "fcntl(F_SETLEASE, F_WRLCK) "
			 "succeeded unexpectedly");
	} else {
		if (TEST_ERRNO == EBUSY || TEST_ERRNO == EAGAIN) {
			tst_resm(TPASS | TTERRNO,
				 "fcntl(F_SETLEASE, F_WRLCK) "
				 "failed as expected");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "fcntl(F_SETLEASE, F_WRLCK) "
				 "failed unexpectedly, "
				 "expected errno is EBUSY or EAGAIN");
		}
	}

	SAFE_CLOSE(cleanup, fd1);
	fd1 = 0;
	SAFE_CLOSE(cleanup, fd2);
	fd2 = 0;
}

static void cleanup(void)
{
	if (fd1 > 0 && close(fd1))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	tst_rmdir();
}

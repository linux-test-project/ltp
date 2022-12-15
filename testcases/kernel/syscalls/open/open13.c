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
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */

/*
 * DESCRIPTION
 *  Basic test for O_PATH flag of open(2).
 *  "Obtain a file descriptor that can be used to perform operations
 *   that act purely at the file descriptor level, the file itself is
 *   not opened, the operations read(2), write(2), fchmod(2), fchown(2)
 *   and fgetxattr(2) fail with the error EBADF."
 *
 *  The operations include but are not limited to the syscalls above.
 */

#define _GNU_SOURCE

#include "config.h"

#include <errno.h>
#ifdef HAVE_SYS_XATTR_H
#include <sys/types.h>
#include <sys/xattr.h>
#endif

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

#define TESTFILE	"testfile"
#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)

static void setup(void);
static void verify_read(void);
static void verify_write(void);
static void verify_fchmod(void);
static void verify_fchown(void);
#ifdef HAVE_SYS_XATTR_H
static void verify_fgetxattr(void);
#endif
static void check_result(const char *call_name);
static void cleanup(void);

static int fd;

static void (*test_func[])(void) = {
	verify_read,
	verify_write,
	verify_fchmod,
	verify_fchown,
#ifdef HAVE_SYS_XATTR_H
	verify_fgetxattr
#endif
};

char *TCID = "open13";
int TST_TOTAL = ARRAY_SIZE(test_func);

int main(int ac, char **av)
{
	int lc;
	int tc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		fd = SAFE_OPEN(cleanup, TESTFILE, O_RDWR | O_PATH);

		for (tc = 0; tc < TST_TOTAL; tc++)
			(*test_func[tc])();

		SAFE_CLOSE(cleanup, fd);
		fd = 0;
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TESTFILE, FILE_MODE, NULL);

	TEST_PAUSE;
}

static void verify_read(void)
{
	char buf[255];

	TEST(read(fd, buf, sizeof(buf)));
	check_result("read(2)");
}

static void verify_write(void)
{
	TEST(write(fd, "w", 1));
	check_result("write(2)");
}

static void verify_fchmod(void)
{
	TEST(fchmod(fd, 0666));
	check_result("fchmod(2)");
}

static void verify_fchown(void)
{
	TEST(fchown(fd, 1000, 1000));
	check_result("fchown(2)");
}

#ifdef HAVE_SYS_XATTR_H
static void verify_fgetxattr(void)
{
	TEST(fgetxattr(fd, "tkey", NULL, 1));
	check_result("fgetxattr(2)");
}
#endif

static void check_result(const char *call_name)
{
	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "%s succeeded unexpectedly", call_name);
		return;
	}

	if (TEST_ERRNO != EBADF) {
		tst_resm(TFAIL | TTERRNO, "%s failed unexpectedly, "
			"expected EBADF", call_name);
		return;
	}

	tst_resm(TPASS, "%s failed with EBADF", call_name);
}

static void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "failed to close file");

	tst_rmdir();
}

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */

/*
 * Test Description :
 *   Testcase to check the basic functionality of the readlink(2),
 *   readlink() will succeed to read the contents of the symbolic link.
 */

#include <pwd.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"

#define TESTFILE "test_file"
#define SYMFILE	"slink_file"

static uid_t nobody_uid;

static void test_readlink(void)
{
	char buffer[256];
	int exp_val = strlen(TESTFILE);

	TEST(readlink(SYMFILE, buffer, sizeof(buffer)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "readlink() on %s failed", SYMFILE);
		return;
	}

	if (TST_RET != exp_val) {
		tst_res(TFAIL, "readlink() returned value %ld "
			"did't match, Expected %d", TST_RET, exp_val);
		return;
	}

	if (memcmp(buffer, TESTFILE, exp_val) != 0) {
		tst_res(TFAIL, "Pathname %s and buffer contents %s differ",
			TESTFILE, buffer);
	} else {
		tst_res(TPASS, "readlink() functionality on '%s' was correct",
			SYMFILE);
	}
}

static void verify_readlink(unsigned int n)
{
	pid_t pid;

	if (n) {
		tst_res(TINFO, "Running test as nobody");
		pid = SAFE_FORK();

		if (!pid) {
			SAFE_SETUID(nobody_uid);
			test_readlink();
			return;
		}
	} else {
		tst_res(TINFO, "Running test as root");
		test_readlink();
	}
}

static void setup(void)
{
	int fd;
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	nobody_uid = pw->pw_uid;

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0444);
	SAFE_CLOSE(fd);
	SAFE_SYMLINK(TESTFILE, SYMFILE);
}

static struct tst_test test = {
	.test = verify_readlink,\
	.tcnt = 2,
	.setup = setup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_tmpdir = 1,
};

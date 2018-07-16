/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 * along with this program;  if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DESCRIPTION
 *	Check if setuid behaves correctly with file permissions. The test
 *	creates a file as ROOT with permissions 0644, does a setuid and then
 *	tries to open the file with RDWR permissions. The same test is done
 *	in a fork to check if new UIDs are correctly passed to the son.
 */

#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "tst_test.h"
#include "compat_tst_16.h"

#define  FILENAME  "setuid04_testfile"

static void dosetuid(void);

static void verify_setuid(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid)
		dosetuid();
	else
		dosetuid();
}

static void dosetuid(void)
{
	int tst_fd;

	TEST(tst_fd = open(FILENAME, O_RDWR));
	if (TST_RET != -1) {
		tst_res(TFAIL, "open() succeeded unexpectedly");
		close(tst_fd);
		return;
	}

	if (TST_ERR == EACCES)
		tst_res(TPASS, "open() returned errno EACCES");
	else
		tst_res(TFAIL | TTERRNO, "open() returned unexpected errno");
}

static void setup(void)
{
	struct passwd *pw;
	uid_t uid;

	pw = SAFE_GETPWNAM("nobody");
	uid = pw->pw_uid;

	UID16_CHECK(uid, setuid);
	/* Create test file */
	SAFE_TOUCH(FILENAME, 0644, NULL);

	if (SETUID(uid) == -1) {
		tst_brk(TBROK,
			"setuid() failed to set the effective uid to %d", uid);
	}
}

static struct tst_test test = {
	.setup = setup,
	.needs_root = 1,
	.test_all = verify_setuid,
	.needs_tmpdir = 1,
	.forks_child = 1,
};

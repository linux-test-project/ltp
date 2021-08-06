// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to check the whether chroot sets errno to EPERM.
 *
 * As a non-root user attempt to perform chroot() to a directory. The
 * chroot() call should fail with EPERM
 */

#include <stdlib.h>
#include <pwd.h>
#include "tst_test.h"

static char *path;

static void verify_chroot(void)
{
	TST_EXP_FAIL(chroot(path), EPERM, "unprivileged chroot()");
}

static void setup(void)
{
	struct passwd *ltpuser;

	path = tst_get_tmpdir();
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void cleanup(void)
{
	free(path);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.setup = setup,
	.test_all = verify_chroot,
	.needs_root = 1,
	.needs_tmpdir = 1,
};

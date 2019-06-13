// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*
 * access(2) test for errno(s) EFAULT as root and nobody respectively.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"

static uid_t uid;

static struct tcase {
	void *addr;
	int mode;
	char *name;
} tcases[] = {
	{(void *)-1, F_OK, "F_OK"},
	{(void *)-1, R_OK, "R_OK"},
	{(void *)-1, W_OK, "W_OK"},
	{(void *)-1, X_OK, "X_OK"},
};

static void access_test(struct tcase *tc, const char *user)
{
	TEST(access(tc->addr, tc->mode));

	if (TST_RET != -1) {
		tst_res(TFAIL, "access(%p, %s) as %s succeeded unexpectedly",
			tc->addr, tc->name, user);
		return;
	}

	if (TST_ERR != EFAULT) {
		tst_res(TFAIL | TTERRNO,
			"access(%p, %s) as %s should fail with EFAULT",
			tc->addr, tc->name, user);
		return;
	}

	tst_res(TPASS | TTERRNO, "access(%p, %s) as %s",
		tc->addr, tc->name, user);
}

static void verify_access(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid;

	/* test as root */
	access_test(tc, "root");

	/* test as nobody */
	pid = SAFE_FORK();
	if (pid) {
		SAFE_WAITPID(pid, NULL, 0);
	} else {
		SAFE_SETUID(uid);
		access_test(tc, "nobody");
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	uid = pw->pw_uid;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test = verify_access,
};

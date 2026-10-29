// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 */

/*\
 * Testcase to check the basic functionality of getgid().
 *
 * [Algorithm]
 *
 * For functionality test the return value from getgid() is compared to passwd
 * entry.
 */

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static void run(void)
{
	uid_t uid;
	struct passwd *pwent;

	TEST(GETGID());
	if (TST_RET < 0)
		tst_brk(TBROK, "This should never happen");

	uid = getuid();
	pwent = getpwuid(uid);
	if (pwent == NULL)
		tst_brk(TBROK | TERRNO, "getuid() returned unexpected value %d", uid);

	GID16_CHECK(pwent->pw_gid, getgid);

	if (pwent->pw_gid != TST_RET) {
		tst_res(TFAIL, "getgid() return value "
				"%ld unexpected - expected %d",
				TST_RET, pwent->pw_gid);
		return;
	}

	tst_res(TPASS, "values from getgid() and getpwuid() match");
}

static struct tst_test test = {
	.test_all = run,
};

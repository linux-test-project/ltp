// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2020
 * Copyright (c) Wipro Technologies Ltd, 2002
 * Author: Subhab Biswas <subhabrata.biswas@wipro.com>
 */

/*\
 * [Description]
 *
 * Test for iopl(2) system call error.
 *
 * - iopl(2) fail with EINVAL when privilege level greater than 3.
 * - iopl(2) fail with EPERM when the current user is not the super-user.
 */

#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#if defined __i386__ || defined(__x86_64__)
#include <sys/io.h>

static struct tcase {
	int level;
	char *desc;
	int exp_errno;
} tcases[] = {
	{4, "Invalid privilege level", EINVAL},
	{1, "Non super-user", EPERM}
};

static void verify_iopl(unsigned int i)
{
	TEST(iopl(tcases[i].level));

	if ((TST_RET == -1) && (TST_ERR == tcases[i].exp_errno)) {
		tst_res(TPASS | TTERRNO,
			"Expected failure for %s, errno: %d",
			tcases[i].desc, TST_ERR);
	} else {
		tst_res(TFAIL | TTERRNO,
			"%s returned %ld expected -1, expected %s got ",
			tcases[i].desc, TST_RET, tst_strerrno(tcases[i].exp_errno));
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pw->pw_uid);
}

static void cleanup(void)
{
	SAFE_SETEUID(0);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_iopl,
	.needs_root = 1,
	/* iopl() is restricted under kernel lockdown. */
	.skip_in_lockdown = 1,
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("LSB v1.3 does not specify iopl() for this architecture. (only for i386 or x86_64)");
#endif /* __i386_, __x86_64__*/

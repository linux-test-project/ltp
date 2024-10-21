// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2007-2018
 * Author: Vatsal Avasthi
 */

/*\
 * [Description]
 *
 * Basic test for flock(2), uses LOCK_SH, LOCK_UN, LOCK_EX locks.
 */

#include <errno.h>
#include <sys/file.h>

#include "tst_test.h"

static int fd = -1;

static struct tcase {
	int operation;
	char *opt;
} tcases[] = {
	{LOCK_SH, "Shared Lock" },
	{LOCK_UN, "Unlock"},
	{LOCK_EX, "Exclusive Lock"},
};

static void verify_flock(unsigned n)
{
	struct tcase *tc = &tcases[n];

	TEST(flock(fd, tc->operation));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO,
			"flock() failed to get %s", tc->opt);
	} else {
		tst_res(TPASS,
			"flock() succeeded with %s", tc->opt);
	}
}

static void setup(void)
{
	fd = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0644);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_flock,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
};

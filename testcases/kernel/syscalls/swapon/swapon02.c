// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * This test case checks whether swapon(2) system call returns
 *  1. ENOENT when the path does not exist
 *  2. EINVAL when the path exists but is invalid
 *  3. EPERM when user is not a superuser
 *  4. EBUSY when the specified path is already being used as a swap area
 */

#include <errno.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

static void setup01(void);
static void cleanup01(void);

static uid_t nobody_uid;
static int do_swapoff;

static struct tcase {
	char *err_desc;
	int exp_errno;
	char *exp_errval;
	char *path;
	void (*setup)(void);
	void (*cleanup)(void);
} tcases[] = {
	{"Path does not exist", ENOENT, "ENOENT", "./doesnotexist", NULL, NULL},
	{"Invalid path", EINVAL, "EINVAL", "./notswap", NULL, NULL},
	{"Permission denied", EPERM, "EPERM", "./swapfile01", setup01, cleanup01},
	{"File already used", EBUSY, "EBUSY", "./alreadyused", NULL, NULL},
};

static void setup01(void)
{
	SAFE_SETEUID(nobody_uid);
}

static void cleanup01(void)
{
	SAFE_SETEUID(0);
}

static void setup(void)
{
	struct passwd *nobody;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_uid = nobody->pw_uid;

	is_swap_supported("./tstswap");

	SAFE_TOUCH("notswap", 0777, NULL);
	make_swapfile("swapfile01", 0);
	make_swapfile("alreadyused", 0);

	if (tst_syscall(__NR_swapon, "alreadyused", 0))
		tst_res(TWARN | TERRNO, "swapon(alreadyused) failed");
	else
		do_swapoff = 1;
}

void cleanup(void)
{
	if (do_swapoff && tst_syscall(__NR_swapoff, "alreadyused"))
		tst_res(TWARN | TERRNO, "swapoff(alreadyused) failed");
}

static void verify_swapon(unsigned int i)
{
	struct tcase *tc = tcases + i;
	if (tc->setup)
		tc->setup();

	TEST(tst_syscall(__NR_swapon, tc->path, 0));

	if (tc->cleanup)
		tc->cleanup();

	if (TST_RET == -1 && TST_ERR == tc->exp_errno) {
		tst_res(TPASS, "swapon(2) expected failure;"
			 " Got errno - %s : %s",
			 tc->exp_errval, tc->err_desc);
		return;
	}

	tst_res(TFAIL, "swapon(2) failed to produce expected error:"
	         " %d, errno: %s and got %d.", tc->exp_errno,
	         tc->exp_errval, TST_ERR);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test = verify_swapon,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup
};

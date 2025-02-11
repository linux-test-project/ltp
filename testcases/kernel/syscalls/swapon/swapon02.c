// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2002-2023
 */

/*\
 * This test case checks whether swapon(2) system call returns:
 *
 * - ENOENT when the path does not exist
 * - EINVAL when the path exists but is invalid
 * - EPERM when user is not a superuser
 * - EBUSY when the specified path is already being used as a swap area
 */

#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/testswap"
#define NOTSWAP_FILE	MNTPOINT"/notswap"
#define SWAP_FILE	MNTPOINT"/swapfile"
#define USED_FILE	MNTPOINT"/alreadyused"

static uid_t nobody_uid;
static int do_swapoff;

static struct tcase {
	char *err_desc;
	int exp_errno;
	char *path;
} tcases[] = {
	{"Path does not exist", ENOENT, "./doesnotexist"},
	{"Invalid path", EINVAL, NOTSWAP_FILE},
	{"Permission denied", EPERM, SWAP_FILE},
	{"File already used", EBUSY, USED_FILE},
};

static void setup(void)
{
	struct passwd *nobody;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_uid = nobody->pw_uid;

	is_swap_supported(TEST_FILE);

	SAFE_TOUCH(NOTSWAP_FILE, 0777, NULL);
	MAKE_SMALL_SWAPFILE(SWAP_FILE);
	MAKE_SMALL_SWAPFILE(USED_FILE);

	if (tst_syscall(__NR_swapon, USED_FILE, 0))
		tst_res(TWARN | TERRNO, "swapon(alreadyused) failed");
	else
		do_swapoff = 1;
}

static void cleanup(void)
{
	if (do_swapoff && tst_syscall(__NR_swapoff, USED_FILE))
		tst_res(TWARN | TERRNO, "swapoff(alreadyused) failed");
}

static void verify_swapon(unsigned int i)
{
	struct tcase *tc = tcases + i;
	if (tc->exp_errno == EPERM)
		SAFE_SETEUID(nobody_uid);

	TST_EXP_FAIL(tst_syscall(__NR_swapon, tc->path, 0), tc->exp_errno,
		     "swapon(2) fail with %s", tc->err_desc);

	if (tc->exp_errno == EPERM)
		SAFE_SETEUID(0);

	if (TST_RET != -1) {
		tst_res(TFAIL, "swapon(2) failed unexpectedly, expected: %s",
			tst_strerrno(tc->exp_errno));
	}
}

static struct tst_test test = {
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
	.test = verify_swapon,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*\
 * name_to_handle_at() tests for AT_HANDLE_FID handles.
 */

#define _GNU_SOURCE
#include <sys/stat.h>
#include "lapi/name_to_handle_at.h"

#define TEST_FILE "test_file"

static int fd_atcwd = AT_FDCWD;
static struct file_handle *fhp;

static struct tcase {
	const char *name;
	int *dfd;
	const char *pathname;
	int name_flags;
	int exp_errno;
} tcases[] = {
	{"test-file", &fd_atcwd, TEST_FILE, AT_HANDLE_FID, 0},
	{"unexportable-file", &fd_atcwd, "/proc/filesystems", AT_HANDLE_FID, 0},
	{"test-file-connectable", &fd_atcwd, TEST_FILE, AT_HANDLE_FID | AT_HANDLE_CONNECTABLE, EINVAL},
};

static bool handle_type_supported(unsigned int flag, const char *name)
{
	/*
	 * For kernels which don't support the flag, name_to_handle_at()
	 * returns EINVAL, otherwise we should get back EBADF because dirfd is
	 * invalid.
	 */
	if (name_to_handle_at(-1, ".", NULL, NULL, flag) && errno == EINVAL) {
		tst_brk(TCONF, "%s not supported by the kernel.", name);
		return false;
	}
	return true;
}

#define REQUIRE_HANDLE_TYPE_SUPPORT(flag) handle_type_supported(flag, #flag)

static void setup(void)
{
	SAFE_TOUCH(TEST_FILE, 0600, NULL);
	fhp = malloc(MAX_HANDLE_SZ);
	if (!fhp)
		tst_brk(TBROK, "malloc failed");

	REQUIRE_HANDLE_TYPE_SUPPORT(AT_HANDLE_FID);
	REQUIRE_HANDLE_TYPE_SUPPORT(AT_HANDLE_CONNECTABLE);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int mount_id;

	fhp->handle_bytes = MAX_HANDLE_SZ;
	TEST(name_to_handle_at(*tc->dfd, tc->pathname, fhp, &mount_id,
			       tc->name_flags));
	if (!tc->exp_errno) {
		if (TST_RET)
			tst_res(TFAIL | TTERRNO, "%s: name_to_handle_at() failed", tc->name);
		else
			tst_res(TPASS, "%s: name_to_handle_at() passed", tc->name);
		return;
	}

	if (TST_RET != -1)
		tst_res(TFAIL, "%s: name_to_handle_at() unexpectedly succeeded", tc->name);
	else if (TST_ERR != tc->exp_errno)
		tst_res(TFAIL | TTERRNO, "%s: name_to_handle_at() should fail with errno %s",
			tc->name, tst_strerrno(tc->exp_errno));
	else
		tst_res(TPASS, "%s: name_to_handle_at() failed as expected", tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "48b77733d0db"},
		{}
	},
};

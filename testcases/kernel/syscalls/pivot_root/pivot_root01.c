// SPDX-License-Identifier: GPL-2.0-or-later
//
// Copyright (c) 2019 Google, Inc.

#define _GNU_SOURCE

#include "config.h"

#include <errno.h>
#include <lapi/syscalls.h>
#include <sched.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/mount.h"

#ifdef HAVE_UNSHARE

#ifdef HAVE_LIBCAP
#include <sys/capability.h>
#endif

#define CHROOT_DIR	"chroot"
#define NEW_ROOT	"/new_root"
#define PUT_OLD		"/new_root/put_old"
#define PUT_OLD_FS	"/put_old_fs"
#define PUT_OLD_BAD	"/put_old_fs/put_old"

enum {
	/*
	 * Test consists of a series of steps that allow pivot_root to succeed,
	 * which is run when param is NORMAL. All other values tweak one of the
	 * steps to induce a failure, and check the errno is as expected.
	 */
	NORMAL,

	/*
	 * EBUSY
	 * new_root or put_old are on the current root file system
	 */
	NEW_ROOT_ON_CURRENT_ROOT,

	/*
	 * EINVAL
	 * put_old is not underneath new_root
	 * Note: if put_old and new_root are on the same fs,
	 * pivot_root fails with EBUSY before testing reachability
	 */
	PUT_OLD_NOT_UNDERNEATH_NEW_ROOT,

	/*
	 * ENOTDIR
	 * new_root or put_old is not a directory
	 */
	PUT_OLD_NOT_DIR,

	/*
	 * EPERM
	 * The calling process does not have the CAP_SYS_ADMIN capability.
	 */
	NO_CAP_SYS_ADMIN,
};

static const struct test_case {
	int test_case;
	int expected_error;
} test_cases[] = {
	{NORMAL, 0},
	{NEW_ROOT_ON_CURRENT_ROOT, EBUSY},
	{PUT_OLD_NOT_UNDERNEATH_NEW_ROOT, EINVAL},
	{PUT_OLD_NOT_DIR, ENOTDIR},
	{NO_CAP_SYS_ADMIN, EPERM},
};

#ifdef HAVE_LIBCAP
static void drop_cap_sys_admin(void)
{
	cap_value_t cap_value[] = { CAP_SYS_ADMIN };
	cap_t cap = cap_get_proc();
	if (!cap)
		tst_brk(TBROK | TERRNO, "cap_get_proc failed");

	if (cap_set_flag(cap, CAP_EFFECTIVE, 1, cap_value, CAP_CLEAR))
		tst_brk(TBROK | TERRNO, "cap_set_flag failed");

	if (cap_set_proc(cap))
		tst_brk(TBROK | TERRNO, "cap_set_proc failed");
}
#endif

static void run(unsigned int test_case)
{
	/* Work in child process - needed to undo unshare and chroot */
	if (SAFE_FORK()) {
		tst_reap_children();
		return;
	}

	/* pivot_root requires no shared mounts exist in process namespace */
	TEST(unshare(CLONE_NEWNS | CLONE_FS));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "unshare failed");

	/*
	 * Create an initial root dir. pivot_root doesn't work if the initial root
	 * dir is a initramfs, so use chroot to create a safe environment
	 */
	SAFE_MOUNT("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);
	SAFE_MOUNT("none", CHROOT_DIR, "tmpfs", 0, 0);
	SAFE_CHROOT(CHROOT_DIR);

	SAFE_MKDIR(NEW_ROOT, 0777);

	/*
	 * pivot_root only works if new_root is a mount point, so mount a tmpfs
	 * unless testing for that fail mode
	 */
	if (test_cases[test_case].test_case != NEW_ROOT_ON_CURRENT_ROOT)
		SAFE_MOUNT("none", NEW_ROOT, "tmpfs", 0, 0);

	/*
	 * Create put_old under new_root, unless testing for that specific fail
	 * mode
	 */
	const char* actual_put_old = NULL;
	if (test_cases[test_case].test_case == PUT_OLD_NOT_UNDERNEATH_NEW_ROOT) {
		actual_put_old = PUT_OLD_BAD;
		SAFE_MKDIR(PUT_OLD_FS, 0777);
		SAFE_MOUNT("none", PUT_OLD_FS, "tmpfs", 0, 0);
		SAFE_MKDIR(PUT_OLD_BAD, 0777);
	} else {
		actual_put_old = PUT_OLD;

		if (test_cases[test_case].test_case == PUT_OLD_NOT_DIR)
			SAFE_CREAT(PUT_OLD, 0777);
		else
			SAFE_MKDIR(PUT_OLD, 0777);
	}

	if (test_cases[test_case].test_case == NO_CAP_SYS_ADMIN) {
#ifdef HAVE_LIBCAP
		drop_cap_sys_admin();
#else
		tst_res(TCONF,
			"System doesn't have POSIX capabilities support");
		return;
#endif
	}

	TEST(syscall(__NR_pivot_root, NEW_ROOT, actual_put_old));

	if (test_cases[test_case].test_case == NORMAL) {
		if (TST_RET)
			tst_res(TFAIL | TTERRNO, "pivot_root failed");
		else
			tst_res(TPASS, "pivot_root succeeded");

		return;
	}

	if (TST_RET == 0) {
		tst_res(TFAIL, "pivot_root succeeded unexpectedly");
		return;
	}

	if (errno != test_cases[test_case].expected_error) {
		tst_res(TFAIL | TERRNO,	"pivot_root failed with wrong errno");
		return;
	}

	tst_res(TPASS | TERRNO, "pivot_root failed as expectedly");
}

static void setup(void)
{
	SAFE_MKDIR(CHROOT_DIR, 0777);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
};

#else
	TST_TEST_TCONF("unshare is undefined.");
#endif

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Failure tests for open_by_handle_at().
 */
#define _GNU_SOURCE
#include <linux/capability.h>
#include "tst_capability.h"
#include "lapi/name_to_handle_at.h"

#define TEST_FILE "test_file"
#define FOO_SYMLINK "foo_symlink"

static struct file_handle high_fh = {.handle_bytes = MAX_HANDLE_SZ + 1}, *high_fhp = &high_fh;
static struct file_handle zero_fh, *zero_fhp = &zero_fh;
static struct file_handle *valid_fhp, *invalid_fhp, *link_fhp;

static struct tst_cap cap_req = TST_CAP(TST_CAP_REQ, CAP_DAC_READ_SEARCH);
static struct tst_cap cap_drop = TST_CAP(TST_CAP_DROP, CAP_DAC_READ_SEARCH);

static struct tcase {
	const char *name;
	int dfd;
	struct file_handle **fhp;
	int flags;
	int cap;
	int exp_errno;
} tcases[] = {
	{"invalid-dfd", -1, &valid_fhp, O_RDWR, 0, EBADF},
	{"stale-dfd", 0, &valid_fhp, O_RDWR, 0, ESTALE},
	{"invalid-file-handle", AT_FDCWD, &invalid_fhp, O_RDWR, 0, EFAULT},
	{"high-file-handle-size", AT_FDCWD, &high_fhp, O_RDWR, 0, EINVAL},
	{"zero-file-handle-size", AT_FDCWD, &zero_fhp, O_RDWR, 0, EINVAL},
	{"no-capability", AT_FDCWD, &valid_fhp, O_RDWR, 1, EPERM},
	{"symlink", AT_FDCWD, &link_fhp, O_RDWR, 0, ELOOP},
};

static void setup(void)
{
	void *faulty_address;
	int mount_id;

	SAFE_TOUCH(TEST_FILE, 0600, NULL);
	SAFE_SYMLINK(TEST_FILE, FOO_SYMLINK);
	faulty_address = tst_get_bad_addr(NULL);
	invalid_fhp = faulty_address;

	valid_fhp = allocate_file_handle(AT_FDCWD, TEST_FILE);
	if (!valid_fhp)
		return;

	TEST(name_to_handle_at(AT_FDCWD, TEST_FILE, valid_fhp, &mount_id, 0));
	if (TST_RET)
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed");

	/* Symlink's file handle */
	link_fhp = tst_alloc(sizeof(*link_fhp) + valid_fhp->handle_bytes);
	link_fhp->handle_type = valid_fhp->handle_type;
	link_fhp->handle_bytes = valid_fhp->handle_bytes;

	TEST(name_to_handle_at(AT_FDCWD, FOO_SYMLINK, link_fhp, &mount_id, 0));
	if (TST_RET)
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd;

	if (tc->cap)
		tst_cap_action(&cap_drop);

	TEST(fd = open_by_handle_at(tc->dfd, *tc->fhp, tc->flags));

	if (tc->cap)
		tst_cap_action(&cap_req);

	if (TST_RET != -1) {
		SAFE_CLOSE(fd);
		tst_res(TFAIL, "%s: open_by_handle_at() passed unexpectedly",
			tc->name);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO,
			"%s: open_by_handle_at() should fail with %s", tc->name,
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: open_by_handle_at() failed as expected",
		tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};

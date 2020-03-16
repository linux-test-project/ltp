// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic openat2() test to check various failures.
 */
#include "tst_test.h"
#include "lapi/openat2.h"

#define TEST_FILE "test_file"

static struct open_how *how;
static struct open_how_pad *phow;

static struct tcase {
	const char *name;
	int dfd;
	const char *pathname;
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
	struct open_how **how;
	size_t size;
	int exp_errno;
} tcases[] = {
	{"invalid-dfd", -1, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, 0, &how, sizeof(*how), EBADF},
	{"invalid-pathname", AT_FDCWD, NULL, O_RDONLY | O_CREAT, S_IRUSR, 0, &how, sizeof(*how), EFAULT},
	{"invalid-flags", AT_FDCWD, TEST_FILE, O_RDONLY, S_IWUSR, 0, &how, sizeof(*how), EINVAL},
	{"invalid-mode", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, -1, 0, &how, sizeof(*how), EINVAL},
	{"invalid-resolve", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, -1, &how, sizeof(*how), EINVAL},
	{"invalid-size-zero", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, 0, &how, 0, EINVAL},
	{"invalid-size-small", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, 0, &how, sizeof(*how) - 1, EINVAL},
	{"invalid-size-big", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, 0, &how, sizeof(*how) + 1, EFAULT},
	{"invalid-size-big-with-pad", AT_FDCWD, TEST_FILE, O_RDWR | O_CREAT, S_IRWXU, 0, (struct open_how **)&phow, sizeof(*how) + 8, E2BIG},
};

static void setup(void)
{
	openat2_supported_by_kernel();
	phow->pad = 0xDEAD;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct open_how *myhow = *tc->how;

	myhow->flags = tc->flags;
	myhow->mode = tc->mode;
	myhow->resolve = tc->resolve;

	TEST(openat2(tc->dfd, tc->pathname, myhow, tc->size));

	if (TST_RET >= 0) {
		SAFE_CLOSE(TST_RET);
		tst_res(TFAIL, "%s: openat2() passed unexpectedly",
			tc->name);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: openat2() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: openat2() failed as expected", tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&how, .size = sizeof(*how)},
		{&phow, .size = sizeof(*phow)},
		{},
	}
};

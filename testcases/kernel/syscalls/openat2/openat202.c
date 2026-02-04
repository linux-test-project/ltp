// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * :manpage:`openat2(2)` tests with various resolve flags.
 */

#define _GNU_SOURCE
#include <fcntl.h>

#include "tst_test.h"
#include "lapi/openat2.h"

#define FOO_SYMLINK "foo_symlink"

static struct open_how *how;

static struct tcase {
	const char *name;
	const char *pathname;
	uint64_t resolve;
	int exp_errno;
} tcases[] = {
	/* Success cases */
	{"open /proc/version", "/proc/version", 0, 0},
	{"open magiclinks", "/proc/self/exe", 0, 0},
	{"open symlinks", FOO_SYMLINK, 0, 0},

	/* Failure cases */
	{"resolve-no-xdev", "/proc/version", RESOLVE_NO_XDEV, EXDEV},
	{"resolve-no-magiclinks", "/proc/self/exe", RESOLVE_NO_MAGICLINKS, ELOOP},
	{"resolve-no-symlinks", FOO_SYMLINK, RESOLVE_NO_SYMLINKS, ELOOP},
	{"resolve-beneath", "/proc/version", RESOLVE_BENEATH, EXDEV},
	{"resolve-beneath", "../foo", RESOLVE_BENEATH, EXDEV},
	{"resolve-no-in-root", "/proc/version", RESOLVE_IN_ROOT, ENOENT},
};

static void setup(void)
{
	openat2_supported_by_kernel();
	SAFE_SYMLINK("/proc/version", FOO_SYMLINK);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	how->flags = O_RDONLY | O_CREAT;
	how->mode = S_IRUSR;
	how->resolve = tc->resolve;

	TEST(openat2(AT_FDCWD, tc->pathname, how, sizeof(*how)));

	if (!tc->exp_errno) {
		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO, "%s: openat2() failed",
				tc->name);
			return;
		}

		SAFE_CLOSE(TST_RET);
		tst_res(TPASS, "%s: openat2() passed", tc->name);
	} else {
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

		tst_res(TPASS | TTERRNO, "%s: openat2() failed as expected",
			tc->name);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&how, .size = sizeof(*how)},
		{},
	}
};

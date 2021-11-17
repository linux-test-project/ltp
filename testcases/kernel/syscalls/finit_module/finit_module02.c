// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Basic finit_module() failure tests.
 *
 * [Algorithm]
 *
 * Tests various failure scenarios for finit_module().
 */

#include <linux/capability.h>
#include <errno.h>
#include "lapi/init_module.h"
#include "tst_module.h"
#include "tst_capability.h"

#define MODULE_NAME	"finit_module.ko"
#define TEST_DIR	"test_dir"

static char *mod_path;

static int fd, fd_zero, fd_invalid = -1, fd_dir;
static int kernel_lockdown;

static struct tst_cap cap_req = TST_CAP(TST_CAP_REQ, CAP_SYS_MODULE);
static struct tst_cap cap_drop = TST_CAP(TST_CAP_DROP, CAP_SYS_MODULE);

struct tcase {
	const char *name;
	int *fd;
	const char *param;
	int open_flags;
	int flags;
	int cap;
	int exp_errno;
	int skip_in_lockdown;
	void (*fix_errno)(struct tcase *tc);
};

static void bad_fd_setup(struct tcase *tc)
{
	if (tst_kvercmp(4, 6, 0) < 0)
		tc->exp_errno = ENOEXEC;
	else
		tc->exp_errno = EBADF;
}

static void dir_setup(struct tcase *tc)
{
	if (tst_kvercmp(4, 6, 0) < 0)
		tc->exp_errno = EISDIR;
	else
		tc->exp_errno = EINVAL;
}

static struct tcase tcases[] = {
	{"invalid-fd", &fd_invalid, "", O_RDONLY | O_CLOEXEC, 0, 0, 0, 0,
		bad_fd_setup},
	{"zero-fd", &fd_zero, "", O_RDONLY | O_CLOEXEC, 0, 0, EINVAL, 0, NULL},
	{"null-param", &fd, NULL, O_RDONLY | O_CLOEXEC, 0, 0, EFAULT, 1, NULL},
	{"invalid-param", &fd, "status=invalid", O_RDONLY | O_CLOEXEC, 0, 0,
		EINVAL, 1, NULL},
	{"invalid-flags", &fd, "", O_RDONLY | O_CLOEXEC, -1, 0, EINVAL, 0,
		NULL},
	{"no-perm", &fd, "", O_RDONLY | O_CLOEXEC, 0, 1, EPERM, 0, NULL},
	{"module-exists", &fd, "", O_RDONLY | O_CLOEXEC, 0, 0, EEXIST, 1,
		NULL},
	{"file-not-readable", &fd, "", O_WRONLY | O_CLOEXEC, 0, 0, EBADF, 0,
		NULL},
	{"file-readwrite", &fd, "", O_RDWR | O_CLOEXEC, 0, 0, ETXTBSY, 0,
		NULL},
	{"directory", &fd_dir, "", O_RDONLY | O_CLOEXEC, 0, 0, 0, 0, dir_setup},
};

static void setup(void)
{
	unsigned long int i;

	finit_module_supported_by_kernel();

	tst_module_exists(MODULE_NAME, &mod_path);

	kernel_lockdown = tst_lockdown_enabled();
	SAFE_MKDIR(TEST_DIR, 0700);
	fd_dir = SAFE_OPEN(TEST_DIR, O_DIRECTORY);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].fix_errno)
			tcases[i].fix_errno(&tcases[i]);
	}
}

static void cleanup(void)
{
	SAFE_CLOSE(fd_dir);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->skip_in_lockdown && kernel_lockdown) {
		tst_res(TCONF, "Kernel is locked down, skipping %s", tc->name);
		return;
	}

	fd = SAFE_OPEN(mod_path, tc->open_flags);

	if (tc->cap)
		tst_cap_action(&cap_drop);

	/* Insert module twice */
	if (tc->exp_errno == EEXIST)
		tst_module_load(MODULE_NAME, NULL);

	TST_EXP_FAIL(finit_module(*tc->fd, tc->param, tc->flags), tc->exp_errno,
		     "TestName: %s", tc->name);

	if (tc->exp_errno == EEXIST)
		tst_module_unload(MODULE_NAME);

	if (!TST_PASS && !TST_RET)
		tst_module_unload(MODULE_NAME);

	if (tc->cap)
		tst_cap_action(&cap_req);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (const struct tst_tag[]) {
		{"linux-git", "032146cda855"},
		{"linux-git", "39d637af5aa7"},
		{}
	},
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};

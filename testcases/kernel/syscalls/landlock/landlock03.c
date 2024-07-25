// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that landlock_restrict_self syscall fails with the right
 * error codes:
 *
 * - EINVAL flags is not 0
 * - EBADF ruleset_fd is not a file descriptor for the current thread
 * - EBADFD ruleset_fd is not a ruleset file descriptor
 * - EPERM ruleset doesn't have CAP_SYS_ADMIN in its namespace
 * - E2BIG The maximum number of stacked rulesets is reached for the current
 *   thread
 */

#include "landlock_common.h"

#define MAX_STACKED_RULESETS 16

static struct landlock_ruleset_attr *ruleset_attr;
static int ruleset_fd = -1;
static int ruleset_invalid = -1;
static int file_fd = -1;

#define ID_NAME(x) .id = x, .name = #x
static struct tst_cap dropadmin = {
	.action = TST_CAP_DROP,
	ID_NAME(CAP_SYS_ADMIN),
};

static struct tst_cap needadmin = {
	.action = TST_CAP_REQ,
	ID_NAME(CAP_SYS_ADMIN),
};

static struct tcase {
	int *fd;
	uint32_t flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{&ruleset_fd, -1, EINVAL, "Invalid flags"},
	{&ruleset_invalid, 0, EBADF, "Invalid file descriptor"},
	{&file_fd, 0, EBADFD, "Not a ruleset file descriptor"},
	{&ruleset_fd, 0, EPERM, "File descriptor doesn't have CAP_SYS_ADMIN"},
	{&ruleset_fd, 0, E2BIG, "Maximum number of stacked rulesets is reached"},
};

static void run_child(struct tcase *tc)
{
	if (tc->exp_errno == EPERM)
		tst_cap_action(&dropadmin);

	if (tc->exp_errno == E2BIG) {
		for (int i = 0; i < MAX_STACKED_RULESETS; i++) {
			TST_EXP_PASS_SILENT(tst_syscall(__NR_landlock_restrict_self,
				*tc->fd, tc->flags));
			if (TST_RET == -1)
				return;
		}
	}

	TST_EXP_FAIL(tst_syscall(__NR_landlock_restrict_self, *tc->fd, tc->flags),
		tc->exp_errno,
		"%s", tc->msg);

	if (tc->exp_errno == EPERM)
		tst_cap_action(&needadmin);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (!SAFE_FORK()) {
		run_child(tc);
		_exit(0);
	}
}

static void setup(void)
{
	verify_landlock_is_enabled();

	ruleset_attr->handled_access_fs = LANDLOCK_ACCESS_FS_EXECUTE;

	ruleset_fd = TST_EXP_FD_SILENT(tst_syscall(__NR_landlock_create_ruleset,
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0));

	file_fd = SAFE_OPEN("junk.bin", O_CREAT, 0777);
}

static void cleanup(void)
{
	if (ruleset_fd != -1)
		SAFE_CLOSE(ruleset_fd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "5.13",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SECURITY_LANDLOCK=y",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};

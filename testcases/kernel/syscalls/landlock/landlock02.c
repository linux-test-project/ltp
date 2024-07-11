// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that landlock_add_rule syscall fails with the right
 * error codes:
 *
 * - EINVAL flags is not 0, or the rule accesses are inconsistent
 * - ENOMSG Empty accesses (i.e., rule_attr->allowed_access is 0)
 * - EBADF ruleset_fd  is  not  a  file  descriptor  for  the  current  thread,
 *   or a member of rule_attr is not a file descriptor as expected
 * - EBADFD ruleset_fd is not a ruleset file descriptor, or a member of
 *   rule_attr is not the expected file descriptor type
 * - EFAULT rule_attr was not a valid address
 */

#include "landlock_common.h"

static struct landlock_ruleset_attr *ruleset_attr;
static struct landlock_path_beneath_attr *path_beneath_attr;
static struct landlock_path_beneath_attr *rule_null;
static int ruleset_fd;
static int invalid_fd = -1;

static struct tcase {
	int *fd;
	enum landlock_rule_type rule_type;
	struct landlock_path_beneath_attr **attr;
	int access;
	int parent_fd;
	uint32_t flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{
		.fd = &ruleset_fd,
		.attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.flags = 1,
		.exp_errno = EINVAL,
		.msg = "Invalid flags"
	},
	{
		.fd = &ruleset_fd,
		.attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.exp_errno = EINVAL,
		.msg = "Invalid rule type"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.attr = &path_beneath_attr,
		.exp_errno = ENOMSG,
		.msg = "Empty accesses"
	},
	{
		.fd = &invalid_fd,
		.attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.exp_errno = EBADF,
		.msg = "Invalid file descriptor"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.parent_fd = -1,
		.exp_errno = EBADF,
		.msg = "Invalid parent fd"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.attr = &rule_null,
		.exp_errno = EFAULT,
		.msg = "Invalid rule attr"
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (*tc->attr) {
		(*tc->attr)->allowed_access = tc->access;
		(*tc->attr)->parent_fd = tc->parent_fd;
	}

	TST_EXP_FAIL(tst_syscall(__NR_landlock_add_rule,
			*tc->fd, tc->rule_type, *tc->attr, tc->flags),
		tc->exp_errno,
		"%s",
		tc->msg);
}

static void setup(void)
{
	verify_landlock_is_enabled();

	ruleset_attr->handled_access_fs = LANDLOCK_ACCESS_FS_EXECUTE;

	ruleset_fd = TST_EXP_FD_SILENT(tst_syscall(__NR_landlock_create_ruleset,
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0));
}

static void cleanup(void)
{
	if (ruleset_fd != -1)
		SAFE_CLOSE(ruleset_fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "5.13",
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SECURITY_LANDLOCK=y",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{&path_beneath_attr, .size = sizeof(struct landlock_path_beneath_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};

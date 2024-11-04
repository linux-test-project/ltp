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

static struct tst_landlock_ruleset_attr_abi1 *attr_abi1;
static struct tst_landlock_ruleset_attr_abi4 *attr_abi4;
static struct landlock_path_beneath_attr *path_beneath_attr;
static struct landlock_path_beneath_attr *rule_null;
static struct landlock_net_port_attr *net_port_attr;
static int ruleset_fd;
static int invalid_fd = -1;
static int abi_current;

static struct tcase {
	int *fd;
	int rule_type;
	struct landlock_path_beneath_attr **path_attr;
	struct landlock_net_port_attr **net_attr;
	int access;
	int parent_fd;
	int net_port;
	uint32_t flags;
	int exp_errno;
	int abi_ver;
	char *msg;
} tcases[] = {
	{
		.fd = &ruleset_fd,
		.path_attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.flags = 1,
		.exp_errno = EINVAL,
		.abi_ver = 1,
		.msg = "Invalid flags"
	},
	{
		.fd = &ruleset_fd,
		.path_attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.exp_errno = EINVAL,
		.abi_ver = 1,
		.msg = "Invalid rule type"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.path_attr = &path_beneath_attr,
		.exp_errno = ENOMSG,
		.abi_ver = 1,
		.msg = "Empty accesses"
	},
	{
		.fd = &invalid_fd,
		.path_attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.exp_errno = EBADF,
		.abi_ver = 1,
		.msg = "Invalid file descriptor"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.path_attr = &path_beneath_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.parent_fd = -1,
		.exp_errno = EBADF,
		.abi_ver = 1,
		.msg = "Invalid parent fd"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_PATH_BENEATH,
		.path_attr = &rule_null,
		.exp_errno = EFAULT,
		.abi_ver = 1,
		.msg = "Invalid rule attr"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_NET_PORT,
		.net_attr = &net_port_attr,
		.access = LANDLOCK_ACCESS_FS_EXECUTE,
		.net_port = 448,
		.exp_errno = EINVAL,
		.abi_ver = 4,
		.msg = "Invalid access rule for network type"
	},
	{
		.fd = &ruleset_fd,
		.rule_type = LANDLOCK_RULE_NET_PORT,
		.net_attr = &net_port_attr,
		.access = LANDLOCK_ACCESS_NET_BIND_TCP,
		.net_port = INT16_MAX + 1,
		.exp_errno = EINVAL,
		.abi_ver = 4,
		.msg = "Socket port greater than 65535"
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	void *attr = NULL;

	if (tc->abi_ver > abi_current) {
		tst_res(TCONF, "Minimum ABI required: %d", tc->abi_ver);
		return;
	}

	if (tc->path_attr && *tc->path_attr) {
		(*tc->path_attr)->allowed_access = tc->access;
		(*tc->path_attr)->parent_fd = tc->parent_fd;

		attr = *tc->path_attr;
	} else if (tc->net_attr && *tc->net_attr) {
		(*tc->net_attr)->allowed_access = tc->access;
		(*tc->net_attr)->port = tc->net_port;

		attr = *tc->net_attr;
	}

	TST_EXP_FAIL(tst_syscall(__NR_landlock_add_rule,
		*tc->fd, tc->rule_type, attr, tc->flags),
		tc->exp_errno, "%s", tc->msg);
}

static void setup(void)
{
	abi_current = verify_landlock_is_enabled();

	attr_abi1->handled_access_fs =
		attr_abi4->handled_access_fs = LANDLOCK_ACCESS_FS_EXECUTE;

	if (abi_current < 4) {
		ruleset_fd = TST_EXP_FD_SILENT(tst_syscall(__NR_landlock_create_ruleset,
			attr_abi1, sizeof(struct tst_landlock_ruleset_attr_abi1), 0));
	} else {
		ruleset_fd = TST_EXP_FD_SILENT(tst_syscall(__NR_landlock_create_ruleset,
			attr_abi4, sizeof(struct tst_landlock_ruleset_attr_abi4), 0));
	}
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
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		{&attr_abi1, .size = sizeof(struct tst_landlock_ruleset_attr_abi1)},
		{&attr_abi4, .size = sizeof(struct tst_landlock_ruleset_attr_abi4)},
		{&path_beneath_attr, .size = sizeof(struct landlock_path_beneath_attr)},
		{&net_port_attr, .size = sizeof(struct landlock_net_port_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};

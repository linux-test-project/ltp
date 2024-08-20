// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/**
 * [Description]
 *
 * CVE-2024-42318
 *
 * Test to check if system is affected by Landlock Houdini bug:
 * https://www.suse.com/security/cve/CVE-2024-42318.html
 *
 * Kernel bug fixed in:
 *
 *  commit 39705a6c29f8a2b93cf5b99528a55366c50014d1
 *  Author: Jann Horn <jannh@google.com>
 *  Date:   Wed Jul 24 14:49:01 2024 +0200
 *
 *  landlock: Don't lose track of restrictions on cred_transfer
 */

#include "tst_test.h"
#include "lapi/keyctl.h"
#include "lapi/prctl.h"
#include "landlock_common.h"

static struct landlock_ruleset_attr *ruleset_attr;
static int ruleset_fd;

static pid_t spawn_houdini(void)
{
	pid_t pid;

	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);
		SAFE_KEYCTL(KEYCTL_SESSION_TO_PARENT, 0, 0, 0, 0);
		exit(0);
	}

	return pid;
}

static void run(void)
{
	pid_t pid_houdini;

	if (SAFE_FORK())
		return;

	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
	SAFE_LANDLOCK_RESTRICT_SELF(ruleset_fd, 0);

	TST_EXP_FAIL(open("/dev/null", O_WRONLY), EACCES);
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		return;
	}

	pid_houdini = spawn_houdini();
	SAFE_WAITPID(pid_houdini, NULL, 0);

	TST_EXP_FAIL(open("/dev/null", O_WRONLY), EACCES);
	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);

	exit(0);
}

static void setup(void)
{
	verify_landlock_is_enabled();

	ruleset_attr->handled_access_fs = LANDLOCK_ACCESS_FS_WRITE_FILE;
	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(
		ruleset_attr,
		sizeof(struct landlock_ruleset_attr),
		0);
}

static void cleanup(void)
{
	if (ruleset_fd != -1)
		SAFE_CLOSE(ruleset_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "39705a6c29f8"},
		{"CVE", "2024-42318"},
		{}
	}
};

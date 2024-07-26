/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LANDLOCK_COMMON_H__
#define LANDLOCK_COMMON_H__

#include "tst_test.h"
#include "lapi/prctl.h"
#include "lapi/fcntl.h"
#include "lapi/landlock.h"

static inline void verify_landlock_is_enabled(void)
{
	int abi;

	abi = tst_syscall(__NR_landlock_create_ruleset,
		NULL, 0, LANDLOCK_CREATE_RULESET_VERSION);

	if (abi < 0) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF, "Landlock is currently disabled. "
				"Please enable it either via CONFIG_LSM or "
				"'lsm' kernel parameter.");
		}

		tst_brk(TBROK | TERRNO, "landlock_create_ruleset error");
	}

	tst_res(TINFO, "Landlock ABI v%d", abi);
}

static inline void apply_landlock_rule(
	struct landlock_path_beneath_attr *path_beneath_attr,
	const int ruleset_fd,
	const int access,
	const char *path)
{
	path_beneath_attr->allowed_access = access;
	path_beneath_attr->parent_fd = SAFE_OPEN(path, O_PATH | O_CLOEXEC);

	SAFE_LANDLOCK_ADD_RULE(
		ruleset_fd,
		LANDLOCK_RULE_PATH_BENEATH,
		path_beneath_attr,
		0);

	SAFE_CLOSE(path_beneath_attr->parent_fd);
}

static inline void enforce_ruleset(const int ruleset_fd)
{
	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
	SAFE_LANDLOCK_RESTRICT_SELF(ruleset_fd, 0);
}

static inline void apply_landlock_layer(
	struct landlock_ruleset_attr *ruleset_attr,
	struct landlock_path_beneath_attr *path_beneath_attr,
	const char *path,
	const int access)
{
	int ruleset_fd;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0);

	apply_landlock_rule(path_beneath_attr, ruleset_fd, access, path);
	enforce_ruleset(ruleset_fd);

	SAFE_CLOSE(ruleset_fd);
}

#endif /* LANDLOCK_COMMON_H__ */

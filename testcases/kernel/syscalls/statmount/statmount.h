/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef STATMOUNT_H
#define STATMOUNT_H

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/mount.h"
#include "lapi/syscalls.h"
#include "tst_safe_stdio.h"

static inline int statmount(uint64_t mnt_id, uint64_t mask, struct statmount *buf,
		     size_t bufsize, unsigned int flags)
{
	struct mnt_id_req req = {
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = mnt_id,
		.param = mask,
	};

	return tst_syscall(__NR_statmount, &req, buf, bufsize, flags);
}

static inline int read_peer_group(const char *path)
{
	FILE *file;
	char line[PATH_MAX];
	char mroot[PATH_MAX];
	int group = -1;

	file = SAFE_FOPEN("/proc/self/mountinfo", "r");

	while (fgets(line, sizeof(line), file)) {
		if (sscanf(line, "%*d %*d %*d:%*d %s %*s %*s shared:%d", mroot, &group) != 2)
			continue;

		if (strcmp(mroot, path) == 0)
			break;
	}

	if (group == -1)
		tst_brk(TBROK, "Can't reed peer group ID for %s", path);

	return group;
}

#endif

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

#endif

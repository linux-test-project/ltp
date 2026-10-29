/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LISTMOUNT_H
#define LISTMOUNT_H

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/mount.h"
#include "lapi/syscalls.h"

static inline ssize_t listmount(uint64_t mnt_id, uint64_t last_mnt_id,
			 uint64_t list[], size_t num, unsigned int flags)
{
	mnt_id_req req = {
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = mnt_id,
		.param = last_mnt_id,
	};

	return tst_syscall(__NR_listmount, &req, list, num, flags);
}

#endif

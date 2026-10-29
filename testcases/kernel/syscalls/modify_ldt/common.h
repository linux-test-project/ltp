/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

#ifndef COMMON_H
#define COMMON_H

#include "tst_test.h"
#include "lapi/ldt.h"

static inline void create_segment(void *seg, size_t size)
{
	struct user_desc entry = {
		.entry_number = 0,
		.base_addr = (unsigned long)seg,
		.limit = size,
		.seg_32bit = 1,
		.contents = 0,
		.read_exec_only = 0,
		.limit_in_pages = 0,
		.seg_not_present = 0,
	};

	SAFE_MODIFY_LDT(1, &entry, sizeof(entry));
}

#endif

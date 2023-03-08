// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static inline int eventfd2(unsigned int count, unsigned int flags)
{
	int ret;

	ret = tst_syscall(__NR_eventfd2, count, flags);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "eventfd2");

	return ret;
}

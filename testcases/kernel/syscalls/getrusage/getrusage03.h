/* SPDX-License-Identifier: GPL-2.0
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (C) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

#ifndef LTP_GETRUSAGE03_H
#define LTP_GETRUSAGE03_H

#include "tst_test.h"

#define DELTA_MAX 20480

static void consume_mb(int consume_nr)
{
	void *ptr;
	size_t size;
	unsigned long vmswap_size;

	mlockall(MCL_CURRENT|MCL_FUTURE);

	size = consume_nr * 1024 * 1024;
	ptr = SAFE_MALLOC(size);
	memset(ptr, 0, size);

	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmSwap: %lu", &vmswap_size);
	if (vmswap_size > 0)
		tst_brk(TBROK, "VmSwap is not zero");
}

static int is_in_delta(long value)
{
	return (value >= -DELTA_MAX && value <= DELTA_MAX);
}

#endif //LTP_GETRUSAGE03_H

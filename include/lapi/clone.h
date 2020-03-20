// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_CLONE_H__
#define LAPI_CLONE_H__

#include <sys/syscall.h>
#include <linux/types.h>
#include <sched.h>

#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_CLONE3
struct clone_args {
	uint64_t __attribute__((aligned(8))) flags;
	uint64_t __attribute__((aligned(8))) pidfd;
	uint64_t __attribute__((aligned(8))) child_tid;
	uint64_t __attribute__((aligned(8))) parent_tid;
	uint64_t __attribute__((aligned(8))) exit_signal;
	uint64_t __attribute__((aligned(8))) stack;
	uint64_t __attribute__((aligned(8))) stack_size;
	uint64_t __attribute__((aligned(8))) tls;
};

int clone3(struct clone_args *args, size_t size)
{
	return tst_syscall(__NR_clone3, args, size);
}
#endif

#ifndef CLONE_PIDFD
#define CLONE_PIDFD	0x00001000	/* set if a pidfd should be placed in parent */
#endif

void clone3_supported_by_kernel(void)
{
	if ((tst_kvercmp(5, 3, 0)) < 0) {
		/* Check if the syscall is backported on an older kernel */
		TEST(syscall(__NR_clone3, NULL, 0));
		if (TST_RET == -1 && TST_ERR == ENOSYS)
			tst_brk(TCONF, "Test not supported on kernel version < v5.3");
	}
}

#endif /* LAPI_CLONE_H__ */

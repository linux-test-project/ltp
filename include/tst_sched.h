/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 */

#ifndef TST_SCHED_H_
#define TST_SCHED_H_

#include <sched.h>

#include "lapi/syscalls.h"

#define TST_LIBC_SCHED_SCALL_(SCALL, ...)({ \
	int tst_ret = SCALL(__VA_ARGS__); \
	if (tst_ret == -1 && errno == ENOSYS) { \
		tst_brk(TCONF, #SCALL " not supported"); \
	} \
	tst_ret; \
})

static inline int sys_sched_setparam(pid_t pid, const struct sched_param *param)
{
	return tst_syscall(__NR_sched_setparam, pid, param);
}

static inline int sys_sched_getparam(pid_t pid, struct sched_param *param)
{
	return tst_syscall(__NR_sched_getparam, pid, param);
}

static inline int sys_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	return tst_syscall(__NR_sched_setscheduler, pid, policy, param);
}

static inline int sys_sched_getscheduler(pid_t pid)
{
	return tst_syscall(__NR_sched_getscheduler, pid);
}

static inline int libc_sched_setparam(pid_t pid, const struct sched_param *param)
{
	return TST_LIBC_SCHED_SCALL_(sched_setparam, pid, param);
}

static inline int libc_sched_getparam(pid_t pid, struct sched_param *param)
{
	return TST_LIBC_SCHED_SCALL_(sched_getparam, pid, param);
}

static inline int libc_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	return TST_LIBC_SCHED_SCALL_(sched_setscheduler, pid, policy, param);
}

static inline int libc_sched_getscheduler(pid_t pid)
{
	return TST_LIBC_SCHED_SCALL_(sched_getscheduler, pid);
}

struct sched_variant {
	char *desc;

	int (*sched_setparam)(pid_t pid, const struct sched_param *param);
	int (*sched_getparam)(pid_t pid, struct sched_param *param);
	int (*sched_setscheduler)(pid_t pid, int policy, const struct sched_param *param);
	int (*sched_getscheduler)(pid_t pid);

} sched_variants[] = {
	{ .sched_setparam = libc_sched_setparam,
	  .sched_getparam = libc_sched_getparam,
	  .sched_setscheduler = libc_sched_setscheduler,
	  .sched_getscheduler = libc_sched_getscheduler,
	  .desc = "libc"
	},
	{ .sched_setparam = sys_sched_setparam,
	  .sched_getparam = sys_sched_getparam,
	  .sched_setscheduler = sys_sched_setscheduler,
	  .sched_getscheduler = sys_sched_getscheduler,
	  .desc = "syscall"
	},
};

#endif /* TST_SCHED_H_ */

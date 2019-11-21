// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef LAPI_SECCOMP_H
#define LAPI_SECCOMP_H

#include <stdint.h>

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#else
/* Valid values for seccomp.mode and prctl(PR_SET_SECCOMP, <mode>) */
# define SECCOMP_MODE_DISABLED   0
# define SECCOMP_MODE_STRICT     1
# define SECCOMP_MODE_FILTER     2

# define SECCOMP_RET_KILL_THREAD  0x00000000U /* kill the thread */
# define SECCOMP_RET_KILL         SECCOMP_RET_KILL_THREAD
# define SECCOMP_RET_ALLOW        0x7fff0000U /* allow */

/**
 * struct seccomp_data - the format the BPF program executes over.
 * @nr: the system call number
 * @arch: indicates system call convention as an AUDIT_ARCH_* value
 *        as defined in <linux/audit.h>.
 * @instruction_pointer: at the time of the system call.
 * @args: up to 6 system call arguments always stored as 64-bit values
 * regardless of the architecture.
 */
struct seccomp_data {
	int nr;
	uint32_t arch;
	uint64_t instruction_pointer;
	uint64_t args[6];
};

#endif /* HAVE_LINUX_SECCOMP_H*/
#endif /* LAPI_SECCOMP_H */

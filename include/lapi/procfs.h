// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Google LLC
 */

#ifndef LAPI_PROCFS_H__
#define LAPI_PROCFS_H__

#include "config.h"
#include <sys/types.h>
#include <sys/procfs.h>
#include "lapi/posix_types.h"

/*
 * Bionic does not provide the core dump note descriptors. The fallbacks
 * follow include/linux/elfcore.h, using the kernel types so that the layout
 * matches the native ABI on every architecture.
 */
#ifndef HAVE_STRUCT_ELF_PRSTATUS
struct elf_prstatus {
	struct elf_siginfo pr_info;
	short pr_cursig;
	unsigned long pr_sigpend;
	unsigned long pr_sighold;
	pid_t pr_pid;
	pid_t pr_ppid;
	pid_t pr_pgrp;
	pid_t pr_sid;
	struct {
		__kernel_long_t tv_sec;
		__kernel_long_t tv_usec;
	} pr_utime, pr_stime, pr_cutime, pr_cstime;
	elf_gregset_t pr_reg;
	int pr_fpvalid;
};
#endif

#ifndef HAVE_STRUCT_ELF_PRPSINFO
struct elf_prpsinfo {
	char pr_state;
	char pr_sname;
	char pr_zomb;
	char pr_nice;
	unsigned long pr_flag;
	__kernel_uid_t pr_uid;
	__kernel_gid_t pr_gid;
	pid_t pr_pid;
	pid_t pr_ppid;
	pid_t pr_pgrp;
	pid_t pr_sid;
	char pr_fname[16];
	char pr_psargs[ELF_PRARGSZ];
};
#endif

#endif /* LAPI_PROCFS_H__ */

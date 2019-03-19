// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 */

#ifndef TGKILL_H
#define TGKILL_H

#include "config.h"
#include "lapi/syscalls.h"

static inline int sys_tgkill(int tgid, int tid, int sig)
{
	return tst_syscall(__NR_tgkill, tgid, tid, sig);
}

static inline pid_t sys_gettid(void)
{
	return tst_syscall(__NR_gettid);
}

#endif /* TGKILL_H */

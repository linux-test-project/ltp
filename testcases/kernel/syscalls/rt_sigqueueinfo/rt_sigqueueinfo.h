// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef __RT_SIGQUEUEINFO_H__
#define __RT_SIGQUEUEINFO_H__

#include "lapi/syscalls.h"

static int sys_rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t *uinfo)
{
	return tst_syscall(__NR_rt_sigqueueinfo, tgid, sig, uinfo);
}

#endif /* __RT_SIGQUEUEINFO_H__ */

// SPDX-License-Identifier: GPL-2.0-or-later
/*
  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_SETNS_H__
#define LAPI_SETNS_H__

#include "config.h"
#include "lapi/syscalls.h"
#include <sched.h>

#ifndef HAVE_SETNS
int setns(int fd, int nstype)
{
	return tst_syscall(__NR_setns, fd, nstype);
}
#endif

#endif /* LAPI_SETNS_H__ */

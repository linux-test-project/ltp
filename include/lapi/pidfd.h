// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LAPI_PIDFD_H__
#define LAPI_PIDFD_H__

#include <fcntl.h>
#ifdef HAVE_SYS_PIDFD_H
# include <sys/pidfd.h>
#endif
#include "config.h"
#include "lapi/syscalls.h"

#ifndef PIDFD_NONBLOCK
#define PIDFD_NONBLOCK O_NONBLOCK
#endif

static inline void pidfd_send_signal_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_send_signal);
}

#ifndef HAVE_PIDFD_SEND_SIGNAL
static inline int pidfd_send_signal(int pidfd, int sig, siginfo_t *info,
				    unsigned int flags)
{
	return tst_syscall(__NR_pidfd_send_signal, pidfd, sig, info, flags);
}
#endif

static inline void pidfd_open_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_open);
}

#ifndef HAVE_PIDFD_OPEN
static inline int pidfd_open(pid_t pid, unsigned int flags)
{
	return tst_syscall(__NR_pidfd_open, pid, flags);
}
#endif

static inline void pidfd_getfd_supported(void)
{
	/* allow the tests to fail early */
	tst_syscall(__NR_pidfd_getfd);
}

#ifndef HAVE_PIDFD_GETFD
static inline int pidfd_getfd(int pidfd, int targetfd, unsigned int flags)
{
	return tst_syscall(__NR_pidfd_getfd, pidfd, targetfd, flags);
}
#endif

#endif /* LAPI_PIDFD_H__ */

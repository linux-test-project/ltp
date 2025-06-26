// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LAPI_PIDFD_H__
#define LAPI_PIDFD_H__

#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>

#ifdef HAVE_SYS_PIDFD_H
# include <sys/pidfd.h>
#endif

#include "config.h"
#include "lapi/syscalls.h"

#ifndef HAVE_STRUCT_PIDFD_INFO
struct pidfd_info {
	uint64_t mask;
	uint64_t cgroupid;
	uint32_t pid;
	uint32_t tgid;
	uint32_t ppid;
	uint32_t ruid;
	uint32_t rgid;
	uint32_t euid;
	uint32_t egid;
	uint32_t suid;
	uint32_t sgid;
	uint32_t fsuid;
	uint32_t fsgid;
	int32_t exit_code;
	uint32_t coredump_mask;
	uint32_t __spare1;
};
#endif

#ifndef PIDFD_NONBLOCK
# define PIDFD_NONBLOCK O_NONBLOCK
#endif

#ifndef PIDFS_IOCTL_MAGIC
# define PIDFS_IOCTL_MAGIC	0xFF
#endif

#ifndef PIDFD_GET_INFO
# define PIDFD_GET_INFO		_IOWR(PIDFS_IOCTL_MAGIC, 11, struct pidfd_info)
#endif

#ifndef PIDFD_INFO_EXIT
# define PIDFD_INFO_EXIT	(1UL << 3)
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

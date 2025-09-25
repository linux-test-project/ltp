/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef IOCTL_PIDFD_H
#define IOCTL_PIDFD_H

#include "tst_test.h"
#include "lapi/pidfd.h"

static inline int ioctl_pidfd_get_info_supported(void)
{
	pid_t pid;
	int pidfd, ret;
	int supported = 0;
	struct pidfd_info info;

	if (tst_kvercmp(6, 13, 0) >= 0)
		return 1;

	memset(&info, 0, sizeof(struct pidfd_info));

	pid = SAFE_FORK();
	if (!pid)
		exit(100);

	pidfd = SAFE_PIDFD_OPEN(pid, 0);

	ret = ioctl(pidfd, PIDFD_GET_INFO, &info);
	SAFE_WAITPID(pid, NULL, 0);

	if (ret != -1)
		supported = 1;

	SAFE_CLOSE(pidfd);
	return supported;
}

static inline int ioctl_pidfd_info_exit_supported(void)
{
	int ret;
	pid_t pid;
	int pidfd;
	int supported = 0;
	struct pidfd_info info;

	if (tst_kvercmp(6, 15, 0) >= 0)
		return 1;

	memset(&info, 0, sizeof(struct pidfd_info));
	info.mask = PIDFD_INFO_EXIT;

	pid = SAFE_FORK();
	if (!pid)
		exit(100);

	pidfd = SAFE_PIDFD_OPEN(pid, 0);
	SAFE_WAITPID(pid, NULL, 0);

	ret = ioctl(pidfd, PIDFD_GET_INFO, &info);
	if (ret == -1) {
		/* - ENOTTY: old kernels not implementing fs/pidfs.c:pidfd_ioctl
		 * - EINVAL: until v6.13 kernel
		 * - ESRCH: all kernels between v6.13 and v6.15
		 */
		if (errno != ENOTTY &&
			errno != EINVAL &&
			errno != ESRCH)
			tst_brk(TBROK | TERRNO, "ioctl error");
	} else {
		if (info.mask & PIDFD_INFO_EXIT)
			supported = 1;
	}

	SAFE_CLOSE(pidfd);

	return supported;
}

#endif

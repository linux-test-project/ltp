/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef IOCTL_PIDFD_H
#define IOCTL_PIDFD_H

#include "tst_test.h"
#include "lapi/pidfd.h"

static inline int ioctl_pidfd_info_exit_supported(void)
{
	int ret = 0;
	pid_t pid;
	int pidfd;
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

	SAFE_IOCTL(pidfd, PIDFD_GET_INFO, &info);
	SAFE_CLOSE(pidfd);

	if (info.mask & PIDFD_INFO_EXIT)
		ret = 1;

	return ret;
}

#endif

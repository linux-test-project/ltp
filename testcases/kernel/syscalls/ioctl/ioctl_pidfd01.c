// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that ioctl() raises the right errors when an application provides
 * the wrong file descriptor.
 */

#include "ioctl_pidfd.h"

static int exp_errnos_num;
static int exp_errnos[] = {
	EINVAL,
	EBADF,
	ENOTTY,
	EACCES,
};

static struct pidfd_info *info;

static void test_bad_pidfd(struct tst_fd *fd_in)
{
	if (fd_in->type == TST_FD_PIDFD) {
		tst_res(TINFO, "Skipping pidfd: SUCCESS");
		return;
	}

	TST_EXP_FAIL_ARR(ioctl(fd_in->fd, PIDFD_GET_INFO, info),
		  exp_errnos, exp_errnos_num,
		  "ioctl(%s, PIDFD_GET_INFO, info)",
		  tst_fd_desc(fd_in));
}

static void run(void)
{
	TST_FD_FOREACH(fd) {
		tst_res(TINFO, "%s -> ...", tst_fd_desc(&fd));
		test_bad_pidfd(&fd);
	}
}

static void setup(void)
{
	if (!ioctl_pidfd_info_exit_supported())
		tst_brk(TCONF, "PIDFD_INFO_EXIT is not supported by ioctl()");

	exp_errnos_num = ARRAY_SIZE(exp_errnos) - 1;

	if (tst_selinux_enforcing())
		exp_errnos_num++;

	info->mask = PIDFD_INFO_EXIT;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&info, .size = sizeof(*info)},
		{}
	}
};

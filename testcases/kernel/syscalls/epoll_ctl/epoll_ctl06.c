// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify :manpage:`epoll_ctl(2)` EPOLL_CTL_ADD behavior across all file
 * descriptor types.
 *
 * The test iterates over all available fd types via TST_FD_FOREACH and
 * attempts EPOLL_CTL_ADD on each. File descriptor types that implement the
 * poll file operation are expected to succeed. The rest must fail with:
 *
 * - EPERM for fds that are valid but lack poll support (regular files,
 *   directories, /dev/zero, /proc files, memfd).
 * - EBADF for fds that are not usable for I/O (O_PATH, open_tree).
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"
#include "tst_fd.h"

static int exp_errno(enum tst_fd_type type)
{
	switch (type) {
	case TST_FD_FILE:
	case TST_FD_DIR:
	case TST_FD_DEV_ZERO:
	case TST_FD_PROC_MAPS:
	case TST_FD_MEMFD:
	case TST_FD_MEMFD_SECRET:
	case TST_FD_FSOPEN:
	case TST_FD_FSPICK:
		return EPERM;
	case TST_FD_PATH:
	case TST_FD_OPEN_TREE:
		return EBADF;
	default:
		return 0;
	}
}

static void run(void)
{
	int efd, err;
	struct epoll_event ev = {.events = EPOLLIN};

	TST_FD_FOREACH(fd) {
		efd = SAFE_EPOLL_CREATE1(0);
		ev.data.fd = fd.fd;
		err = exp_errno(fd.type);

		if (err) {
			TST_EXP_FAIL(epoll_ctl(efd, EPOLL_CTL_ADD,
				     fd.fd, &ev), err,
				     "epoll_ctl() on %s", tst_fd_desc(&fd));
		} else {
			TST_EXP_PASS(epoll_ctl(efd, EPOLL_CTL_ADD,
				     fd.fd, &ev),
				     "epoll_ctl() on %s", tst_fd_desc(&fd));
		}

		SAFE_CLOSE(efd);
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};

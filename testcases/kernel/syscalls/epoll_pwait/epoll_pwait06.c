// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Verify that various timeout values don't get misinterpreted as infinity
 * by epoll_pwait() and epoll_pwait2(). Regression fixed in:
 *
 * commit d9ec73301099ec5975505e1c3effbe768bab9490
 * Author: Max Kellermann <max.kellermann@ionos.com>
 * Date:   Tue Apr 29 20:58:27 2025 +0200
 *
 * fs/eventpoll: fix endless busy loop after timeout has expired
 */

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_epoll.h"
#include "epoll_pwait_var.h"

static int efd = -1;

static void run(void)
{
	struct timespec timeout = {};
	struct epoll_event e = {};

	e.events = EPOLLIN;

	TST_FD_FOREACH(fd_in) {
		/* File descriptor types not supported by epoll */
		switch (fd_in.type) {
		case TST_FD_FILE:
		case TST_FD_PATH:
		case TST_FD_DIR:
		case TST_FD_DEV_ZERO:
		case TST_FD_PROC_MAPS:
		case TST_FD_FSOPEN:
		case TST_FD_FSPICK:
		case TST_FD_OPEN_TREE:
		case TST_FD_MEMFD:
		case TST_FD_MEMFD_SECRET:
			continue;
		default:
			break;
		}

		tst_res(TINFO, "Testing %s", tst_fd_desc(&fd_in));
		timeout.tv_nsec = 1000000000;
		SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, fd_in.fd, &e);

		do {
			alarm(1);
			timeout.tv_nsec /= 10;
			do_epoll_pwait(efd, &e, 1, &timeout, NULL);
			alarm(0);
		} while (timeout.tv_nsec);

		SAFE_EPOLL_CTL(efd, EPOLL_CTL_DEL, fd_in.fd, &e);
	}

	tst_res(TPASS, "Timeout works correctly");
}

static void setup(void)
{
	epoll_pwait_init();
	efd = SAFE_EPOLL_CREATE1(0);
}

static void cleanup(void)
{
	if (efd >= 0)
		SAFE_CLOSE(efd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = TEST_VARIANTS,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d9ec73301099"},
		{}
	}
};

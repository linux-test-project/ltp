// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` with EPOLLPRI detects changes to
 * /proc/mounts.
 *
 * The kernel generates a poll notification (POLLPRI/EPOLLERR) on
 * /proc/mounts whenever the mount namespace changes. This test verifies
 * that epoll can observe these notifications.
 *
 * [Algorithm]
 *
 * - Open /proc/mounts and register it with an epoll instance for EPOLLPRI.
 * - Fork a child that mounts and then unmounts a tmpfs filesystem, using
 *   checkpoints to synchronize with the parent.
 * - After each mount namespace change the parent calls epoll_wait and verifies
 *   it returns an event.
 *
 * Needs root to call mount/umount.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"

#define MNTPOINT "mnt_epoll"

static int efd = -1, mnt_fd = -1;

static void setup(void)
{
	SAFE_MKDIR(MNTPOINT, 0777);

	mnt_fd = SAFE_OPEN("/proc/mounts", O_RDONLY);

	efd = SAFE_EPOLL_CREATE1(0);

	struct epoll_event ev = {
		.events = EPOLLPRI,
		.data.fd = mnt_fd,
	};

	SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, mnt_fd, &ev);
}

static void check_epoll_event(const char *desc)
{
	struct epoll_event ret_ev;
	char buf[1024];

	TEST(epoll_wait(efd, &ret_ev, 1, 1000));
	if (TST_RET < 1) {
		tst_res(TFAIL | TTERRNO,
			"epoll_wait() after %s returned %li", desc, TST_RET);
	} else {
		TST_EXP_EXPR(ret_ev.events & EPOLLPRI);
	}

	/* Re-read /proc/self/mounts to clear the notification */
	SAFE_LSEEK(mnt_fd, 0, SEEK_SET);
	while (SAFE_READ(0, mnt_fd, buf, sizeof(buf)) > 0)
		;
}

static void run(void)
{
	if (!SAFE_FORK()) {
		SAFE_MOUNT("none", MNTPOINT, "tmpfs", 0, NULL);
		TST_CHECKPOINT_WAKE_AND_WAIT(0);
		SAFE_UMOUNT(MNTPOINT);
		TST_CHECKPOINT_WAKE(0);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	check_epoll_event("mount");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);
	check_epoll_event("umount");
}

static void cleanup(void)
{
	if (efd != -1)
		SAFE_CLOSE(efd);

	if (mnt_fd != -1)
		SAFE_CLOSE(mnt_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *const []){
		"CONFIG_TMPFS",
		NULL
	},
};

// SPDX-License-Identifier: GPL-2.0
/*
 * Taken from the kernel self tests, which in turn were based on
 * a Syzkaller reproducer.
 *
 * Self test author and close_range author:
 * Christian Brauner <christian.brauner@ubuntu.com>
 *
 * LTP Author: Richard Palethorpe <rpalethorpe@suse.com>
 * Copyright (c) 2021 SUSE LLC, other copyrights may apply.
 */
/*\
 * [Description]
 *
 * We check that close_range()
 *
 * - closes FDs
 * - UNSHARES some FDs before closing them
 * - it sets CLOEXEC (in both cloned process and parent)
 * - combination of CLOEXEC and UNSHARE.
 *
 * The final test is the actual bug reproducer. Note that we call
 * clone directly to share the file table.
 */

#include <stdlib.h>

#include "tst_test.h"
#include "tst_clone.h"

#include "lapi/sched.h"
#include "lapi/close_range.h"

static int fd[3];

static inline void do_close_range(unsigned int fd, unsigned int max_fd,
				  unsigned int flags)
{
	int ret = close_range(fd, max_fd, flags);

	if (!ret)
		return;

	if (errno == EINVAL) {
		if (flags & CLOSE_RANGE_UNSHARE)
			tst_brk(TCONF | TERRNO, "No CLOSE_RANGE_UNSHARE");
		if (flags & CLOSE_RANGE_CLOEXEC)
			tst_brk(TCONF | TERRNO, "No CLOSE_RANGE_CLOEXEC");
	}

	tst_brk(TBROK | TERRNO, "close_range(%d, %d, %d)", fd, max_fd, flags);
}

static void setup(void)
{
	close_range_supported_by_kernel();

	struct rlimit nfd;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &nfd);

	if (nfd.rlim_max < 1000) {
		tst_brk(TCONF, "NOFILE limit max too low: %lu < 1000",
			nfd.rlim_max);
	}

	nfd.rlim_cur = nfd.rlim_max;
	SAFE_SETRLIMIT(RLIMIT_NOFILE, &nfd);
}

static void check_cloexec(int i, int expected)
{
	int present = SAFE_FCNTL(fd[i], F_GETFD) & FD_CLOEXEC;

	if (expected && !present)
		tst_res(TFAIL, "fd[%d] flags do not contain FD_CLOEXEC", i);

	if (!expected && present)
		tst_res(TFAIL, "fd[%d] flags contain FD_CLOEXEC", i);
}

static void check_closed(int min)
{
	int i;

	for (i = min; i < 3; i++) {
		if (fcntl(fd[i], F_GETFD) > -1)
			tst_res(TFAIL, "fd[%d] is still open", i);
	}
}

static void child(unsigned int n)
{
	switch (n) {
	case 0:
		SAFE_DUP2(fd[1], fd[2]);
		do_close_range(3, ~0U, 0);
		check_closed(0);
		break;
	case 1:
		SAFE_DUP2(fd[1], fd[2]);
		do_close_range(3, ~0U, CLOSE_RANGE_UNSHARE);
		check_closed(0);
		break;
	case 2:
		do_close_range(3, ~0U, CLOSE_RANGE_CLOEXEC);
		check_cloexec(0, 1);
		check_cloexec(1, 1);

		SAFE_DUP2(fd[1], fd[2]);
		check_cloexec(2, 0);
		break;
	case 3:
		do_close_range(3, ~0U,
			       CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);
		check_cloexec(0, 1);
		check_cloexec(1, 1);

		SAFE_DUP2(fd[1], fd[2]);
		check_cloexec(2, 0);
		break;
	}

	exit(0);
}

static void run(unsigned int n)
{
	const struct tst_clone_args args = {
		.flags = CLONE_FILES,
		.exit_signal = SIGCHLD,
	};

	switch (n) {
	case 0:
		tst_res(TINFO, "Plain close range");
		do_close_range(3, ~0U, 0);
		break;
	case 1:
		tst_res(TINFO, "Set UNSHARE and close range");
		do_close_range(3, ~0U, CLOSE_RANGE_UNSHARE);
		break;
	case 2:
		tst_res(TINFO, "Set CLOEXEC on range");
		do_close_range(3, ~0U, CLOSE_RANGE_CLOEXEC);
		break;
	case 3:
		tst_res(TINFO, "Set UNSHARE and CLOEXEC on range");
		do_close_range(3, ~0U,
			       CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);
		break;
	}

	fd[0] = SAFE_OPEN("mnt/tmpfile", O_RDWR | O_CREAT, 0644);
	fd[1] = SAFE_DUP2(fd[0], 1000);
	fd[2] = 42;

	if (!SAFE_CLONE(&args))
		child(n);

	tst_reap_children();

	switch (n) {
	case 0:
		check_closed(0);
		break;
	case 1:
		check_cloexec(0, 0);
		check_cloexec(1, 0);
		check_cloexec(2, 0);
		break;
	case 2:
		check_cloexec(0, 1);
		check_cloexec(1, 1);
		check_cloexec(2, 0);
		break;
	case 3:
		check_cloexec(0, 0);
		check_cloexec(1, 0);
		check_closed(2);
		break;
	}

	do_close_range(3, ~0U, 0);
	check_closed(0);

	if (tst_taint_check())
		tst_res(TFAIL, "Kernel tainted");
	else
		tst_res(TPASS, "No kernel taints");
}

static struct tst_test test = {
	.tcnt = 4,
	.forks_child = 1,
	.mount_device = 1,
	.mntpoint = "mnt",
	.all_filesystems = 1,
	.needs_root = 1,
	.test = run,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "fec8a6a69103"},
		{},
	},
};

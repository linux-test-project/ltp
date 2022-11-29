// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Federico Bonfiglio <fedebonfi95@gmail.com>
 * Copyright (c) Linux Test Project, 2019-2022
 */

/*\
 * [Description]
 *
 * Test ioctl_ns with NS_GET_USERNS request.
 *
 * After the call to clone with the CLONE_NEWUSER flag,
 * child is created in a new user namespace. That's checked by
 * comparing its /proc/self/ns/user symlink and the parent's one,
 * which should be different.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/ioctl_ns.h"
#include "lapi/sched.h"

#define STACK_SIZE (1024 * 1024)

static char *child_stack;

static void setup(void)
{
	int exists = access("/proc/self/ns/user", F_OK);

	if (exists < 0)
		tst_res(TCONF, "namespace not available");

	child_stack = ltp_alloc_stack(STACK_SIZE);
	if (!child_stack)
		tst_brk(TBROK|TERRNO, "stack alloc");
}

static void cleanup(void)
{
	free(child_stack);
}

static int child(void *arg LTP_ATTRIBUTE_UNUSED)
{
	TST_CHECKPOINT_WAIT(0);
	return 0;
}

static void run(void)
{
	char child_namespace[30];

	pid_t pid = ltp_clone(CLONE_NEWUSER | SIGCHLD, &child, 0,
		STACK_SIZE, child_stack);
	if (pid == -1)
		tst_brk(TBROK | TERRNO, "ltp_clone failed");

	snprintf(child_namespace, sizeof(child_namespace), "/proc/%i/ns/user", pid);
	int my_fd, child_fd, parent_fd;

	my_fd = SAFE_OPEN("/proc/self/ns/user", O_RDONLY);
	child_fd = SAFE_OPEN(child_namespace, O_RDONLY);
	parent_fd = ioctl(child_fd, NS_GET_USERNS);

	if (parent_fd == -1) {
		TST_CHECKPOINT_WAKE(0);

		if (errno == ENOTTY)
			tst_brk(TCONF, "ioctl(NS_GET_USERNS) not implemented");

		tst_brk(TBROK | TERRNO, "ioctl(NS_GET_USERNS) failed");
	}

	struct stat my_stat, child_stat, parent_stat;

	SAFE_FSTAT(my_fd, &my_stat);
	SAFE_FSTAT(child_fd, &child_stat);
	SAFE_FSTAT(parent_fd, &parent_stat);
	if (my_stat.st_ino != parent_stat.st_ino)
		tst_res(TFAIL, "parents have different inodes");
	else if (parent_stat.st_ino == child_stat.st_ino)
		tst_res(TFAIL, "child and parent have same inode");
	else
		tst_res(TPASS, "child and parent are consistent");
	SAFE_CLOSE(my_fd);
	SAFE_CLOSE(parent_fd);
	SAFE_CLOSE(child_fd);
	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.min_kver = "4.9",
	.setup = setup,
	.cleanup = cleanup,
};
